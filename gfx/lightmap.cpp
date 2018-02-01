#include "lightmapper.h"

#include "gl/fbo.h"
#include "gl/shaders.h"
#include "gl/primitive.h"
#include "gl/gpu_clock.h"

namespace pt
{
namespace gfx
{

struct Lightmap::Data
{
    Data() :
        rect(squareMesh()),
        vsQuadUv(gl::Shader::path("quad_uv.vs.glsl")),
        fsLightmapper(gl::Shader::path("lightmapper.fs.glsl")),
        fsVisibility(gl::Shader::path("visibility.fs.glsl")),
        fsUpscale(gl::Shader::path("lightmapper_upscale.fs.glsl")),
        fsCommon(gl::Shader::path("common.fs.glsl")),
        prog({vsQuadUv, fsLightmapper, fsVisibility, fsCommon},
             {{0, "position"}, {1, "uv"}}),
        progHq({vsQuadUv, fsUpscale, fsCommon},
               {{0, "position"}, {1, "uv"}}),
        light     {gl::Texture::Type::Texture3d, gl::Texture::Type::Texture3d},
        incidence {gl::Texture::Type::Texture3d, gl::Texture::Type::Texture3d},
        density   {gl::Texture::Type::Texture3d},
        emission  {gl::Texture::Type::Texture3d},
        emitters  {gl::Texture::Type::Buffer}
    {}

    operator bool() const
    {
        return light.first;
    }

    Lightmap::Size size() const
    {
        return light.first.size();
    }

    // Primitive
    gl::Primitive     rect;

    // Shaders
    gl::Shader        vsQuadUv,
                      fsLightmapper,
                      fsVisibility,
                      fsUpscale,
                      fsCommon;

    // Shader programs
    gl::ShaderProgram prog,
                      progHq;

    // Current lightmap textures, normal/high quality
    mutable Tex       light, incidence;

    // Work buffers for lightmap computation
    gl::Texture       density, emission, emitters;
};

Lightmap::Lightmap() :
    d(std::make_shared<Data>())
{}

Lightmap::Tex& Lightmap::light() const
{
    return d->light;
}

Lightmap::Tex& Lightmap::incidence() const
{
    return d->incidence;
}

Lightmap::Size Lightmap::size() const
{
    return d->size();
}

Lightmap& Lightmap::resize(const Size& size)
{
    if (glm::any(glm::notEqual(d->size(), size)))
    {
        const auto sizeHq = 4 * size;
        const std::vector<int> dims   = {size.x,     size.y,   size.z};
        const std::vector<int> dimsHq = {sizeHq.x, sizeHq.y, sizeHq.z};
        for (auto texPair : {&d->light, &d->incidence})
        {
            texPair->first.bind().alloc(dims, GL_RGBA16F, GL_RGBA, GL_FLOAT)
                                 .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                                 .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
                                 .set(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            texPair->second.bind().alloc(dimsHq, GL_RGBA16F, GL_RGBA, GL_FLOAT)
                                  .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                                  .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
                                  .set(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        }
    }
    return *this;
}

Lightmap& Lightmap::update(const mat::Density& density,
                           const mat::Emission& emission,
                           const Emitters& emitters,
                           const Horizon& horizon)
{
    if (*d)
    {
        // Create emitter buffer
        gl::Buffer emittersBuf(gl::Buffer::Type::Texture);
        emittersBuf.alloc(emitters.data(), sizeof(Emitter) * emitters.size());

        // Upload buffers
        d->density.bind().alloc(density);
        d->emission.bind().alloc(emission);
        d->emitters.bind().alloc(GL_RGBA16I, emittersBuf);
        {
            const Time<GpuClock> clock;
            const auto size = d->light.first.size();

            // Bake pass
            gl::Fbo fbo;
            Binder<gl::Fbo> fboBinder(&fbo);
            Binder<gl::ShaderProgram> progBinder(&d->prog);
            d->prog.setUniform("density",  0)
                   .setUniform("emission", 1)
                   .setUniform("horizon",  2)
                   .setUniform("lightSrc", 3)
                   .setUniform("lsc",      int(emitters.size()))
                   .setUniform("attMin",   0.001f)
                   .setUniform("k0",       1.f)
                   .setUniform("k1",       0.22f)
                   .setUniform("k2",       0.2f)
                   .setUniform("cs",       c::cell::SIZE.xzy());

            const GLenum buffers[] = {GL_COLOR_ATTACHMENT0,
                                      GL_COLOR_ATTACHMENT1};

            glViewport(0, 0, size.x, size.y);
            glDrawBuffers(2, buffers);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);

            d->density.bindAs(GL_TEXTURE0);
            d->emission.bindAs(GL_TEXTURE1);
            horizon.texture().bindAs(GL_TEXTURE2);
            d->emitters.bindAs(GL_TEXTURE3);

            // Z-layers
            for (int z = 0; z < size.z; ++z)
            {
                constexpr auto attachment = gl::Fbo::Attachment::Color;
                fbo.attach(d->light.first,     attachment, 0, 0, z);
                fbo.attach(d->incidence.first, attachment, 1, 0, z);
                d->prog.setUniform("wz", z);
                d->rect.render();
            }

            #if 0
            const auto elapsed = boost::chrono::duration<float, boost::milli>
                                (clock.elapsed()).count();
            const auto vol     = size.x * size.y * size.z;
            PTLOG(Info) << "elapsed " << elapsed << " ms, "
                        << (vol / elapsed) << " cells/ms";
            #endif
        }
        {
            // Upscale HQ textures
            const Time<GpuClock> clockHq;
            const auto size = d->light.second.size();

            gl::Fbo fbo;
            Binder<gl::Fbo> fboBinder(&fbo);
            Binder<gl::ShaderProgram> progBinder(&d->progHq);
            d->progHq.setUniform("texGi",  0)
                     .setUniform("texInc", 1);

            const GLenum buffers[] = {GL_COLOR_ATTACHMENT0,
                                      GL_COLOR_ATTACHMENT1};

            glViewport(0, 0, size.x, size.y);
            glDrawBuffers(2, buffers);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);

            d->light.first.bindAs(GL_TEXTURE0);
            d->incidence.first.bindAs(GL_TEXTURE1);

            // Z-layers
            for (int z = 0; z < size.z; ++z)
            {
                constexpr auto attachment = gl::Fbo::Attachment::Color;
                fbo.attach(d->light.second,     attachment, 0, 0, z);
                fbo.attach(d->incidence.second, attachment, 1, 0, z);
                d->progHq.setUniform("z", float(z) / (size.z - 1));
                d->rect.render();
            }

            #if 0
            const auto elapsed = boost::chrono::duration<float, boost::milli>
                                (clockHq.elapsed()).count();
            const auto vol     = size.x * size.y * size.z;
            PTLOG(Info) << "HQ elapsed " << elapsed << " ms, "
                        << "size: " << size.x << "x" << size.y << "x" << size.z
                        << ", " << (vol / elapsed) << " cells/ms";
            #endif
        }
    }
    return *this;
}

} // namespace gfx
} // namespace pt
