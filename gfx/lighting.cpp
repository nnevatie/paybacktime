#include "lighting.h"

#include <glbinding/gl/bitfield.h>

#include <glm/matrix.hpp>

#include "common/common.h"
#include "common/log.h"

namespace pt
{
namespace gfx
{

struct Lighting::Data
{
    Data() : texScOut(nullptr)
    {}

    // TODO: Move internals here
    gl::Texture* texScOut;
};

Lighting::Lighting(const cfg::Video& config, const gl::Texture& texDepth) :
    d(std::make_shared<Data>()),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsSc(gl::Shader::path("lighting_scattering.fs.glsl")),
    fsOut(gl::Shader::path("lighting.fs.glsl")),
    fsCommon(gl::Shader::path("common.fs.glsl")),
    progSc({vsQuad, fsSc, fsCommon},
           {{0, "position"}, {1, "uv"}}),
    progOut({vsQuad, fsOut, fsCommon},
            {{0, "position"}, {1, "uv"}}),
    blurSc(Size<int>(config.sc.scale * config.output.renderSize())),
    scSampleCount(config.sc.samples)
{
    // Texture and FBO
    auto size    = config.output.renderSize();
    auto sizeSc  = {int(config.sc.scale * size.x), int(config.sc.scale * size.y)};
    auto sizeOut = {int(size.x), int(size.y)};

    texSc.bind().alloc(sizeSc, GL_RGB16F, GL_RGB, GL_FLOAT)
                .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    texOut.bind().alloc(sizeOut, GL_RGB16F, GL_RGB, GL_FLOAT)
                 .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                 .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    fboSc.bind()
         .attach(texSc, gl::Fbo::Attachment::Color)
         .unbind();
    fboOut.bind()
          .attach(texDepth, gl::Fbo::Attachment::Depth)
          .attach(texOut,   gl::Fbo::Attachment::Color)
          .unbind();
}

Lighting& Lighting::sc(gl::Texture* texDepth,
                       gl::Texture* texLightmap,
                       const Camera& camera,
                       const Box& bounds)
{
    // Scattering pass
    Binder<gl::Fbo> binder(fboSc);
    progSc.bind().setUniform("texDepth",    0)
                 .setUniform("texGi",       1)
                 .setUniform("w",           camera.matrixWorld())
                 .setUniform("camPos",      camera.position())
                 .setUniform("boundsMin",   glm::floor(bounds.pos))
                 .setUniform("boundsSize",  glm::ceil(bounds.size))
                 .setUniform("sampleCount", scSampleCount);

    const auto size = texSc.size();
    glViewport(0, 0, size.x, size.y);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    texDepth->bindAs(GL_TEXTURE0);
    texLightmap->bindAs(GL_TEXTURE1);
    rect.render();

    d->texScOut = &texSc;
    if (float(texSc.size().x) / texOut.size().x < 1.f)
    {
        blurSc(&texSc, nullptr, 3);
        d->texScOut = &blurSc.output();
    }
    return *this;
}

Lighting& Lighting::operator()(
    gl::Texture* texDepth,
    gl::Texture* texNormal,
    gl::Texture* texColor,
    gl::Texture* texLight,
    gl::Texture* texSsao,
    gl::Texture* texLightmap,
    gl::Texture* texIncidence,
    const Camera& camera,
    const Box& bounds)
{
    // Combine pass
    Binder<gl::Fbo> binder(fboOut);
    progOut.bind().setUniform("texDepth",    0)
                  .setUniform("texNormal",   1)
                  .setUniform("texColor",    2)
                  .setUniform("texLight",    3)
                  .setUniform("texAo",       4)
                  .setUniform("texSc",       5)
                  .setUniform("texGi",       6)
                  .setUniform("texIncid",    7)
                  .setUniform("z",           0.f)
                  .setUniform("tanHalfFov",  camera.tanHalfFov())
                  .setUniform("aspectRatio", camera.ar)
                  .setUniform("w",           camera.matrixWorld())
                  .setUniform("n",           camera.matrixNormal())
                  .setUniform("boundsMin",   glm::floor(bounds.pos))
                  .setUniform("boundsSize",  glm::ceil(bounds.size));

    const auto size = texOut.size();
    glViewport(0, 0, size.x, size.y);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    texDepth->bindAs(GL_TEXTURE0);
    texNormal->bindAs(GL_TEXTURE1);
    texColor->bindAs(GL_TEXTURE2);
    texLight->bindAs(GL_TEXTURE3);
    texSsao->bindAs(GL_TEXTURE4);
    d->texScOut->bindAs(GL_TEXTURE5);
    texLightmap->bindAs(GL_TEXTURE6);
    texIncidence->bindAs(GL_TEXTURE7);
    rect.render();
    return *this;
}

gl::Texture* Lighting::output()
{
    return &texOut;
}

} // namespace gfx
} // namespace pt
