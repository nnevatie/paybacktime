#include "lightmapper.h"

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>

#include "common/log.h"
#include "gl/fbo.h"
#include "gl/gpu_clock.h"

#include "constants.h"

namespace pt
{
namespace gfx
{
namespace
{

/*
    struct Compare : std::binary_function<glm::ivec3, glm::ivec3, bool>
    {
        bool operator()(const glm::ivec3& lhs, const glm::ivec3 rhs) const
        {
            return (lhs.y < rhs.y) || ((lhs.y == rhs.y) && (lhs.x < rhs.x));
        }
    };
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

Lightmapper::Lightmapper() :
    rect(squareMesh()),
    vsQuadUv(gl::Shader::path("quad_uv.vs.glsl")),
    fsLightmapper(gl::Shader::path("lightmapper.fs.glsl")),
    prog({vsQuadUv, fsLightmapper},
         {{0, "position"}, {1, "uv"}}),
    texLight(gl::Texture::Type::Texture3d),
    texIncidence(gl::Texture::Type::Texture3d),
    texDensity(gl::Texture::Type::Texture3d),
    texEmission(gl::Texture::Type::Texture3d)
{
}

Lightmapper& Lightmapper::reset(const glm::ivec3& size)
{
    if (glm::any(glm::notEqual(texLight.size(), size)))
    {
        const std::vector<int> dims = {size.x, size.y, size.z};

        texLight.bind().alloc(dims, GL_RGB32F, GL_RGB, GL_FLOAT)
                       .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                       .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        texIncidence.bind().alloc(dims, GL_RGB32F, GL_RGB, GL_FLOAT)
                           .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                           .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        PTLOG(Info) << "alloc " << size.x << ", " << size.y << ", " << size.z;
    }
    density  = mat::Density(size);
    emission = mat::Emission(size);
    return *this;
}

Lightmapper& Lightmapper::add(const glm::ivec3& pos,
                              const mat::Density& density,
                              const mat::Emission& emission)
{
    accumulate(this->density, this->emission, pos, density, emission);
    return *this;
}

Lightmapper& Lightmapper::operator()()
{
    PTTIMEU_GPU("generate lightmap", boost::milli);
    if (texLight)
    {
        const auto size = texLight.size();
        texDensity.bind().alloc(density);
        texEmission.bind().alloc(emission);

        gl::Fbo fbo;
        Binder<gl::Fbo> fboBinder(&fbo);
        Binder<gl::ShaderProgram> progBinder(&prog);
        prog.setUniform("density",  0)
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

        texDensity.bindAs(GL_TEXTURE0);
        texEmission.bindAs(GL_TEXTURE1);

        glViewport(0, 0, size.x, size.y);

        for (int z = 0; z < size.z; ++z)
        {
            fbo.attach(texLight,     gl::Fbo::Attachment::Color, 0, 0, z);
            fbo.attach(texIncidence, gl::Fbo::Attachment::Color, 1, 0, z);
            prog.setUniform("wz", z);
            rect.render();

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
