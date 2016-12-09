#include "lightmapper.h"

#include <set>

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>

#include "common/log.h"

#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"
#include "gl/gpu_clock.h"

#include "constants.h"

namespace pt
{
namespace gfx
{
namespace
{

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

/*
    std::set<glm::ivec3, Compare> lightSources;

    lightSources.insert({x1, y1, z1});
*/

void accumulate(
    mat::Density& d0, mat::Emission& e0,
    const glm::ivec3& pos, const mat::Density& d1, const mat::Emission& e1)
{
    auto const size = d1.size;
    for (int z = 0; z < size.z; ++z)
    {
        const auto z1 = pos.z + z;
        for (int y = 0; y < size.y; ++y)
        {
            const auto y1 = pos.y + y;
            for (int x = 0; x < size.x; ++x)
            {
                const auto x1 = pos.x + x;
                d0.at(x1, y1, z1) = glm::min(1.f, d0.at(x1, y1, z1) +
                                                  d1.at(x,  y,  z));
                const auto em1 = e1.at(x, y);
                if (em1 != glm::zero<glm::vec3>())
                {
                    auto em0 = e0.at(x1, y1, z1);
                    auto em2 = em0 + em1;
                    e0.at(x1, y1, z1) = em2;
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
        prog({vsQuadUv, fsLightmapper},
             {{0, "position"}, {1, "uv"}}),
        texLight(gl::Texture::Type::Texture3d),
        texIncidence(gl::Texture::Type::Texture3d),
        texDensity(gl::Texture::Type::Texture3d),
        texEmission(gl::Texture::Type::Texture3d)
    {}

    gl::Primitive     rect;

    gl::Shader        vsQuadUv,
                      fsLightmapper;

    gl::ShaderProgram prog;

    mat::Density      density;
    mat::Emission     emission;

    gl::Texture       texLight,
                      texIncidence,
                      texDensity,
                      texEmission;
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
                          .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        d->texIncidence.bind().alloc(dims, GL_RGB32F, GL_RGB, GL_FLOAT)
                              .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                              .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        PTLOG(Info) << "alloc " << size.x << ", " << size.y << ", " << size.z;
    }
    d->density  = mat::Density(size);
    d->emission = mat::Emission(size);
    return *this;
}

Lightmapper& Lightmapper::add(const glm::ivec3& pos,
                              const mat::Density& density,
                              const mat::Emission& emission)
{
    accumulate(d->density, d->emission, pos, density, emission);
    return *this;
}

Lightmapper& Lightmapper::operator()()
{
    PTTIMEU_GPU("generate lightmap", boost::milli);
    if (d->texLight)
    {
        const auto size = d->texLight.size();
        d->texDensity.bind().alloc(d->density);
        d->texEmission.bind().alloc(d->emission);

        gl::Fbo fbo;
        Binder<gl::Fbo> fboBinder(&fbo);
        Binder<gl::ShaderProgram> progBinder(&d->prog);
        d->prog.setUniform("density",  0)
               .setUniform("emission", 1)
               .setUniform("exp",      1.f)
               .setUniform("ambient",  0.f)
               .setUniform("attMin",   0.005f)
               .setUniform("k0",       1.f)
               .setUniform("k1",       0.5f)
               .setUniform("k2",       0.05f)
               .setUniform("cs",       c::cell::SIZE.xzy());

        const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, buffers);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);

        d->texDensity.bindAs(GL_TEXTURE0);
        d->texEmission.bindAs(GL_TEXTURE1);

        glViewport(0, 0, size.x, size.y);

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
    }
    return *this;
}

} // namespace gfx
} // namespace pt
