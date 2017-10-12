#include "lightmapper.h"

#include <set>
#include <vector>

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/string_cast.hpp>

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
    const Transform& xform,
    const mat::Density& density1,
    const mat::Emission& emission1)
{
    const auto pos     = xform.pos.xzy() +
                         glm::vec3(0.f, 0.f, 0.5f * glm::vec3(density1.size).z);
    const auto rot     = Transform::rotation(c::grid::UP, xform.rot);
    const auto aabb    = density1.bounds(pos).rotated(c::grid::UP, xform.rot);
    const auto size0   = glm::ivec3(glm::ceil(aabb.size()));
    const auto size1   = density1.size;
    const auto min0    = glm::max(glm::zero<glm::ivec3>(),
                                  glm::ivec3(glm::floor(aabb.min)));
    const auto max0    = glm::min(density0.size,
                                  glm::ivec3(glm::ceil(aabb.max)));
    const auto origin0 = pos - glm::vec3(0.5f);
    const auto origin1 = 0.5f * glm::vec3(size1) - glm::vec3(0.5f);

    #if 0
    PTLOG(Info) << "pos: " << glm::to_string(pos)
                << ", aabb: " << glm::to_string(aabb.min)
                << " -> " << glm::to_string(aabb.max)
                << ", min0: " << glm::to_string(min0)
                << ", max0: " << glm::to_string(max0)
                << ", size0: " << glm::to_string(size0)
                << ", size1: " << glm::to_string(size1)
                << ", origin0: " << glm::to_string(origin0)
                << ", origin1: " << glm::to_string(origin1);
    #endif
    glm::vec3 p0;
    for (p0.z = min0.z; p0.z < max0.z; ++p0.z)
        for (p0.y = min0.y; p0.y < max0.y; ++p0.y)
            for (p0.x = min0.x; p0.x < max0.x; ++p0.x)
            {
                const auto xv = glm::vec3(rot * glm::vec4(p0 - origin0, 1.f));
                const auto p1 = glm::ivec3(glm::round(origin1 + xv));

                if (glm::all(glm::greaterThanEqual(p1, glm::zero<glm::ivec3>())) &&
                    glm::all(glm::lessThan(p1, size1)))
                {
                    #if 0
                    PTLOG(Info) << "p0: "   << glm::to_string(p0)
                                << ", p1: " << glm::to_string(p1)
                                << ", " << glm::to_string(origin1 + xv);
                    #endif

                    auto& d0        = density0.at(p0);
                    const auto& d1  = density1.at(p1);

                    const auto aMin = std::min(d0.a, d1.a);
                    const auto a    = aMin < 0.f ? aMin : std::min(1.f, d0.a + d1.a);
                    const auto rgb  = d0.rgb() + d1.rgb();

                    d0 = glm::vec4(rgb, a);

                    const auto em1 = emission1.at(p1);
                    if (em1 != glm::zero<glm::vec3>())
                    {
                        auto em0 = emission0.at(p0);
                        emission0.at(p0) = em0 + em1;
                        lightSources.insert(p0);
                    }
                }
            }

    #if 0
    for (int z = 0; z < density1.size.z; ++z)
        image(density1, z).write("c:/temp/density/src_" +
                                 std::to_string(z) + ".png");

    for (int z = 0; z < density0.size.z; ++z)
        image(density0, z).write("c:/temp/density/dest_" +
                                 std::to_string(z) + ".png");
    #endif
}

} // namespace

struct Lightmapper::Data
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
        texLight(gl::Texture::Type::Texture3d),
        texLightHq(gl::Texture::Type::Texture3d),
        texIncidence(gl::Texture::Type::Texture3d),
        texIncidenceHq(gl::Texture::Type::Texture3d),
        texDensity(gl::Texture::Type::Texture3d),
        texEmission(gl::Texture::Type::Texture3d),
        texLightSources(gl::Texture::Type::Buffer)
    {}

    gl::Primitive     rect;

    gl::Shader        vsQuadUv,
                      fsLightmapper,
                      fsVisibility,
                      fsUpscale,
                      fsCommon;

    gl::ShaderProgram prog,
                      progHq;

    mat::Density      density;
    mat::Emission     emission;
    LightSources      lightSources;

    gl::Texture       texLight,
                      texLightHq,
                      texIncidence,
                      texIncidenceHq,
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
    return &d->texLightHq;
}

gl::Texture* Lightmapper::incidenceTexture() const
{
    return &d->texIncidenceHq;
}

Lightmapper& Lightmapper::reset(const glm::ivec3& size)
{
    if (glm::any(glm::notEqual(d->texLight.size(), size)))
    {
        const std::vector<int> dims = {size.x, size.y, size.z};

        d->texLight.bind().alloc(dims, GL_RGB16F, GL_RGB, GL_FLOAT)
                          .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                          .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
                          .set(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        d->texIncidence.bind().alloc(dims, GL_RGB16F, GL_RGB, GL_FLOAT)
                              .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                              .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
                              .set(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        // HQ versions
        const int scaleHq = 4;
        const auto sizeHq = scaleHq * size;
        const std::vector<int> dimsHq = {sizeHq.x, sizeHq.y, sizeHq.z};

        d->texLightHq.bind().alloc(dimsHq, GL_RGB16F, GL_RGB, GL_FLOAT)
                            .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                            .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
                            .set(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        d->texIncidenceHq.bind().alloc(dimsHq, GL_RGB16F, GL_RGB, GL_FLOAT)
                                .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                                .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
                                .set(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    }
    d->density      = mat::Density(size);
    d->emission     = mat::Emission(size);
    d->lightSources = {};
    return *this;
}

Lightmapper& Lightmapper::add(const Transform& xform,
                              const mat::Density& density,
                              const mat::Emission& emission)
{
    accumulate(d->density, d->emission, d->lightSources,
               xform, density, emission);
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

        // Upload lighting textures
        gl::Buffer lightSourcesBuf(gl::Buffer::Type::Texture);
        lightSourcesBuf.alloc(lightSources.data(),
                              sizeof(LightSource) * lightSourceCount);
        d->texDensity.bind().alloc(d->density);
        d->texEmission.bind().alloc(d->emission);
        d->texLightSources.bind().alloc(GL_RGBA16I, lightSourcesBuf);
        {
            // Bake pass
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

            const GLenum buffers[] = {GL_COLOR_ATTACHMENT0,
                                      GL_COLOR_ATTACHMENT1};

            glViewport(0, 0, size.x, size.y);
            glDrawBuffers(2, buffers);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);

            d->texDensity.bindAs(GL_TEXTURE0);
            d->texEmission.bindAs(GL_TEXTURE1);
            horizon.texture().bindAs(GL_TEXTURE2);
            d->texLightSources.bindAs(GL_TEXTURE3);

            // Z-layers
            for (int z = 0; z < size.z; ++z)
            {
                fbo.attach(d->texLight,     gl::Fbo::Attachment::Color, 0, 0, z);
                fbo.attach(d->texIncidence, gl::Fbo::Attachment::Color, 1, 0, z);
                d->prog.setUniform("wz", z);
                d->rect.render();
            }

            #if 1
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
            const auto sizeHq = d->texLightHq.size();

            gl::Fbo fbo;
            Binder<gl::Fbo> fboBinder(&fbo);
            Binder<gl::ShaderProgram> progBinder(&d->progHq);
            d->progHq.setUniform("texGi",  0)
                     .setUniform("texInc", 1);

            const GLenum buffers[] = {GL_COLOR_ATTACHMENT0,
                                      GL_COLOR_ATTACHMENT1};

            glViewport(0, 0, sizeHq.x, sizeHq.y);
            glDrawBuffers(2, buffers);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);

            d->texLight.bindAs(GL_TEXTURE0);
            d->texIncidence.bindAs(GL_TEXTURE1);

            // Z-layers
            for (int z = 0; z < sizeHq.z; ++z)
            {
                fbo.attach(d->texLightHq,     gl::Fbo::Attachment::Color, 0, 0, z);
                fbo.attach(d->texIncidenceHq, gl::Fbo::Attachment::Color, 1, 0, z);
                d->progHq.setUniform("z", float(z) / (sizeHq.z - 1));
                d->rect.render();
            }

            #if 1
            const auto elapsed = boost::chrono::duration<float, boost::milli>
                                (clockHq.elapsed()).count();
            const auto vol     = sizeHq.x * sizeHq.y * sizeHq.z;
            PTLOG(Info) << "HQ elapsed " << elapsed << " ms, "
                        << "size: " << sizeHq.x << "x" << sizeHq.y << "x" << sizeHq.z
                        << ", " << (vol / elapsed) << " cells/ms";
            #endif
        }
    }
    return *this;
}

} // namespace gfx
} // namespace pt
