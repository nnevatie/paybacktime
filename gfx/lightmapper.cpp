#include "lightmapper.h"

#include "common/log.h"
#include "gl/fbo.h"
#include "gl/gpu_clock.h"

#include "constants.h"

namespace pt
{
namespace gfx
{

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

Lightmapper& Lightmapper::operator()(mat::Light& lightmap,
                                     const mat::Density& density,
                                     const mat::Emission& emission)
{
    PTTIMEU_GPU("generate lightmap", boost::milli);
    if (lightmap)
    {
        if (glm::any(glm::notEqual(texLight.size(), lightmap.size)))
        {
            const auto dims = lightmap.dims();

            texLight.bind().alloc(dims, GL_RGB32F, GL_RGB, GL_FLOAT)
                           .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                           .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            texIncidence.bind().alloc(dims, GL_RGB32F, GL_RGB, GL_FLOAT)
                               .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                               .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            PTLOG(Info) << " alloc "
                        << lightmap.size.x << ", "
                        << lightmap.size.y << ", "
                        << lightmap.size.z;
        }
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

        texDensity.bindAs(GL_TEXTURE0);
        texEmission.bindAs(GL_TEXTURE1);

        for (int z = 0; z < lightmap.size.z; ++z)
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
