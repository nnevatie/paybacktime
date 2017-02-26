#include "lighting.h"

#include <glbinding/gl/bitfield.h>

#include <glm/matrix.hpp>

#include "common/common.h"
#include "common/log.h"

namespace pt
{
namespace gfx
{

Lighting::Lighting(const Size<int>& renderSize, const gl::Texture& texDepth) :
    renderSize(renderSize),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsGi(gl::Shader::path("lighting_gi.fs.glsl")),
    fsSc(gl::Shader::path("lighting_scattering.fs.glsl")),
    fsOut(gl::Shader::path("lighting.fs.glsl")),
    fsCommon(gl::Shader::path("common.fs.glsl")),
    progGi({vsQuad, fsGi, fsCommon},
           {{0, "position"}, {1, "uv"}}),
    progSc({vsQuad, fsSc, fsCommon},
           {{0, "position"}, {1, "uv"}}),
    progOut({vsQuad, fsOut, fsCommon},
            {{0, "position"}, {1, "uv"}})
{
    // Texture and FBO
    auto sizeGi  = {renderSize.w / 2, renderSize.h / 2};
    auto sizeSc  = {renderSize.w / 4, renderSize.h / 4};
    auto sizeOut = {renderSize.w,     renderSize.h};

    texGi.bind().alloc(sizeGi, GL_RGB32F, GL_RGB, GL_FLOAT)
                .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    texSc.bind().alloc(sizeSc, GL_RGB32F, GL_RGB, GL_FLOAT)
                .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    texOut.bind().alloc(sizeOut, GL_RGB32F, GL_RGB, GL_FLOAT)
                 .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                 .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    fboGi.bind()
         .attach(texGi, gl::Fbo::Attachment::Color)
         .unbind();
    fboSc.bind()
         .attach(texSc, gl::Fbo::Attachment::Color)
         .unbind();
    fboOut.bind()
          .attach(texDepth, gl::Fbo::Attachment::Depth)
          .attach(texOut,   gl::Fbo::Attachment::Color)
          .unbind();
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
    const Box& bounds,
    const glm::mat4& v,
    const glm::mat4& p)
{
    {
        // GI pass
        Binder<gl::Fbo> binder(fboGi);
        progGi.bind().setUniform("texDepth",   0)
                     .setUniform("texGi",      1)
                     .setUniform("w",          camera.matrixWorld())
                     .setUniform("boundsMin",  glm::floor(bounds.pos))
                     .setUniform("boundsSize", glm::ceil(bounds.size));

        const auto size = texGi.size();
        glViewport(0, 0, size.x, size.y);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);
        texDepth->bindAs(GL_TEXTURE0);
        texLightmap->bindAs(GL_TEXTURE1);
        rect.render();
    }
    {
        // Scattering pass
        Binder<gl::Fbo> binder(fboSc);
        progSc.bind().setUniform("texDepth",   0)
                     .setUniform("texGi",      1)
                     .setUniform("w",          camera.matrixWorld())
                     .setUniform("camPos",     camera.position())
                     .setUniform("boundsMin",  glm::floor(bounds.pos))
                     .setUniform("boundsSize", glm::ceil(bounds.size));

        const auto size = texSc.size();
        glViewport(0, 0, size.x, size.y);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);
        texDepth->bindAs(GL_TEXTURE0);
        texLightmap->bindAs(GL_TEXTURE1);
        rect.render();
    }
    {
        // Combine pass
        Binder<gl::Fbo> binder(fboOut);
        progOut.bind().setUniform("texDepth",    0)
                      .setUniform("texNormal",   1)
                      .setUniform("texColor",    2)
                      .setUniform("texLight",    3)
                      .setUniform("texAo",       4)
                      .setUniform("texGi",       5)
                      .setUniform("texSc",       6)
                      .setUniform("texIncid",    7)
                      .setUniform("z",           0.f)
                      .setUniform("tanHalfFov",  camera.tanHalfFov())
                      .setUniform("aspectRatio", camera.ar)
                      .setUniform("w",           camera.matrixWorld())
                      .setUniform("n",           camera.matrixNormal())
                      .setUniform("boundsMin",   glm::floor(bounds.pos))
                      .setUniform("boundsSize",  glm::ceil(bounds.size));

        glViewport(0, 0, renderSize.w, renderSize.h);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);
        texDepth->bindAs(GL_TEXTURE0);
        texNormal->bindAs(GL_TEXTURE1);
        texColor->bindAs(GL_TEXTURE2);
        texLight->bindAs(GL_TEXTURE3);
        texSsao->bindAs(GL_TEXTURE4);
        texGi.bindAs(GL_TEXTURE5);
        texSc.bindAs(GL_TEXTURE6);
        texIncidence->bindAs(GL_TEXTURE7);
        rect.render();
    }
    return *this;
}

gl::Texture* Lighting::output()
{
    return &texOut;
}

} // namespace gfx
} // namespace pt
