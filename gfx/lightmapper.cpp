#include "lightmapper.h"

#include <set>
#include <vector>

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>

#include "common/log.h"

#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/buffers.h"
#include "gl/fbo.h"
#include "gl/gpu_clock.h"

#include "constants.h"

namespace pt
{
namespace gfx
{
namespace
{

struct LightSource
{
    int16_t x, y, z, w;
};
struct LightSourceCmp : std::binary_function<glm::ivec3, glm::ivec3, bool>
{
    bool operator()(const glm::ivec3& lhs, const glm::ivec3 rhs) const
    {
        return (lhs.x  < rhs.x)                  ||
               (lhs.x == rhs.x && lhs.y < rhs.y) ||
               (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z < rhs.z);
    }
};
using LightSources = std::set<glm::ivec3, LightSourceCmp>;

void accumulate(
    mat::Density& density0,
    mat::Emission& emission0,
    LightSources& lightSources,
    const glm::ivec3& pos,
    const mat::Density& density1,
    const mat::Emission& emission1)
{
    auto const size = density1.size;
    for (int z = 0; z < size.z; ++z)
    {
        const auto z1 = pos.z + z;
        for (int y = 0; y < size.y; ++y)
        {
            const auto y1 = pos.y + y;
            for (int x = 0; x < size.x; ++x)
            {
                const auto x1   = pos.x + x;
                auto& d0        = density0.at(x1, y1, z1);
                const auto& d1  = density1.at(x,  y,  z);

                const auto aMin = std::min(d0.a, d1.a);
                const auto a    = aMin < 0.f ? aMin : std::min(1.f, d0.a + d1.a);
                const auto rgb  = 0.5f * (d0.rgb() + d1.rgb());

                d0 = glm::vec4(rgb, a);

                const auto em1 = emission1.at(x, y, z);
                if (em1 != glm::zero<glm::vec3>())
                {
                    auto em0 = emission0.at(x1, y1, z1);
                    emission0.at(x1, y1, z1) = em0 + em1;
                    lightSources.insert({x1, y1, z1});
                }

            }
        }
    }
}

} // namespace

struct Lightmapper::Data
{
    Data() :
        rect(squareMesh()),
        vsQuadUv(gl::Shader::path("quad_uv.vs.glsl")),
        fsLightmapper(gl::Shader::path("lightmapper.fs.glsl")),
        fsCommon(gl::Shader::path("common.fs.glsl")),
        prog({vsQuadUv, fsLightmapper, fsCommon},
             {{0, "position"}, {1, "uv"}}),
        texLight(gl::Texture::Type::Texture3d),
        texIncidence(gl::Texture::Type::Texture3d),
        texDensity(gl::Texture::Type::Texture3d),
        texEmission(gl::Texture::Type::Texture3d),
        texLightSources(gl::Texture::Type::Buffer)
    {}

    gl::Primitive     rect;

    gl::Shader        vsQuadUv,
                      fsLightmapper,
                      fsCommon;

    gl::ShaderProgram prog;

    mat::Density      density;
    mat::Emission     emission;
    LightSources      lightSources;

    gl::Texture       texLight,
                      texIncidence,
                      texDensity,
                      texEmission,
                      texLightSources;
};

Lightmapper::Lightmapper() :
    d(std::make_shared<Data>())
{
}

gl::Texture* Lightmapper::lightTexture() const
{
    return &d->texLight;
}

gl::Texture* Lightmapper::incidenceTexture() const
{
    return &d->texIncidence;
}

Lightmapper& Lightmapper::reset(const glm::ivec3& size)
{
    if (glm::any(glm::notEqual(d->texLight.size(), size)))
    {
        const std::vector<int> dims = {size.x, size.y, size.z};

        d->texLight.bind().alloc(dims, GL_RGB32F, GL_RGB, GL_FLOAT)
                          .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                          .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
                          .set(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        d->texIncidence.bind().alloc(dims, GL_RGB32F, GL_RGB, GL_FLOAT)
                              .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                              .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
                              .set(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    }
    d->density      = mat::Density(size);
    d->emission     = mat::Emission(size);
    d->lightSources = {};
    return *this;
}

Lightmapper& Lightmapper::add(const glm::ivec3& pos,
                              const mat::Density& density,
                              const mat::Emission& emission)
{
    accumulate(d->density, d->emission, d->lightSources, pos, density, emission);
    return *this;
}

Lightmapper& Lightmapper::operator()(const Horizon& horizon)
{
    if (d->texLight)
    {
        const Time<GpuClock> clock;
        const auto size = d->texLight.size();

        // Populate lightsources buffer
        const int lightSourceCount = d->lightSources.size();
        std::vector<LightSource> lightSources;
        lightSources.reserve(lightSourceCount);
        for (const auto& ls : d->lightSources)
            lightSources.push_back({int16_t(ls.x),
                                    int16_t(ls.y),
                                    int16_t(ls.z), 0});

        gl::Buffer lightSourcesBuf(gl::Buffer::Type::Texture);
        lightSourcesBuf.alloc(lightSources.data(),
                              sizeof(LightSource) * lightSourceCount);

        d->texDensity.bind().alloc(d->density);
        d->texEmission.bind().alloc(d->emission);
        d->texLightSources.bind().alloc(GL_RGBA16I, lightSourcesBuf);

        gl::Fbo fbo;
        Binder<gl::Fbo> fboBinder(&fbo);
        Binder<gl::ShaderProgram> progBinder(&d->prog);
        d->prog.setUniform("density",  0)
               .setUniform("emission", 1)
               .setUniform("horizon",  2)
               .setUniform("lightSrc", 3)
               .setUniform("lsc",      lightSourceCount)
               .setUniform("attMin",   0.001f)
               .setUniform("k0",       1.f)
               .setUniform("k1",       0.22f)
               .setUniform("k2",       0.2f)
               .setUniform("cs",       c::cell::SIZE.xzy());

        const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, buffers);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);

        d->texDensity.bindAs(GL_TEXTURE0);
        d->texEmission.bindAs(GL_TEXTURE1);
        if (horizon) horizon.texture().bindAs(GL_TEXTURE2);
        d->texLightSources.bindAs(GL_TEXTURE3);

        glViewport(0, 0, size.x, size.y);

        #if 0
        mat::Light lightmap(size);
        #endif

        for (int z = 0; z < size.z; ++z)
        {
            fbo.attach(d->texLight,     gl::Fbo::Attachment::Color, 0, 0, z);
            fbo.attach(d->texIncidence, gl::Fbo::Attachment::Color, 1, 0, z);
            d->prog.setUniform("wz", z);
            d->rect.render();

            #if 0
            glReadPixels(0, 0, lightmap.size.x, lightmap.size.y,
                         GL_RGB, GL_FLOAT, static_cast<GLvoid*>(
                                               &lightmap.at(0, 0, z)));
            #endif
        }

        #if 1
        const auto elapsed = boost::chrono::duration<float, boost::milli>
                            (clock.elapsed()).count();
        const auto vol     = size.x * size.y * size.z;

        PTLOG(Info) << "elapsed " << elapsed << " ms, "
                    << (vol / elapsed) << " cells/ms";
        #endif

        #if 0
        for (int z = 0; z < size.z; ++z)
        {
            const auto zs = std::to_string(z);
            image(lightmap, z).write("c:/temp/" + zs + "_light.png");
            image(d->density, z).write("c:/temp/" + zs + "_density.png");
            image(d->emission, z).write("c:/temp/" + zs + "_emission.png");
        }
        #endif
    }
    return *this;
}

} // namespace gfx
} // namespace pt
