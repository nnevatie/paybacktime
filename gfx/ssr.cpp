#include "ssr.h"

#include <glm/gtx/transform.hpp>

#include "platform/clock.h"
#include "common/log.h"

namespace pt
{
namespace gfx
{

Ssr::Ssr(const Size<int>& displaySize, const Size<int>& renderSize) :
    displaySize(displaySize),
    renderSize(renderSize),
    scale(float(renderSize.w) / displaySize.w),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsCommon(gl::Shader::path("common.fs.glsl")),
    fsSsr(gl::Shader::path("ssr.fs.glsl")),
    fsComp(gl::Shader::path("ssr_composite.fs.glsl")),
    progSsr({vsQuad, fsSsr, fsCommon},
           {{0, "position"}, {1, "uv"}}),
    progComp({vsQuad, fsComp},
            {{0, "position"}, {1, "uv"}})
{
    // SSR
    auto ssrSize = {renderSize.w, renderSize.h};
    texSsr.bind().alloc(ssrSize, GL_RGB16F, GL_RGB, GL_FLOAT);
    fboSsr.bind()
          .attach(texSsr, gl::Fbo::Attachment::Color)
          .unbind();

    // Composite
    auto outputSize = {displaySize.w, displaySize.h};
    texComp.bind().alloc(outputSize, GL_RGB16F, GL_RGB, GL_FLOAT);
    fboComp.bind()
           .attach(texComp, gl::Fbo::Attachment::Color)
           .unbind();
}

Ssr& Ssr::operator()(gl::Texture* texDepth,
                     gl::Texture* texNormal,
                     gl::Texture* texColor,
                     gl::Texture* texLight,
                     gl::Texture* texEnv,
                     const Camera& camera)
{
    {
        // SSR
        Binder<gl::Fbo> binder(fboSsr);
        fboSsr.attach(texSsr, gl::Fbo::Attachment::Color);

        // View-to-screen transformation
        auto sc0 = glm::scale({}, glm::vec3(displaySize.w, displaySize.h, 1));
        auto sc1 = glm::scale({}, glm::vec3(0.5f, 0.5f, 1));
        auto tr  = glm::translate({}, glm::vec3(0.5f, 0.5f, 0));
        auto pc  = (sc0 * (tr * sc1)) * camera.matrixProj();

        progSsr.bind().setUniform("texDepth",    0)
                      .setUniform("texNormal",   1)
                      .setUniform("z",           0.f)
                      .setUniform("tanHalfFov",  std::tan(0.5f * camera.fov))
                      .setUniform("aspectRatio", camera.ar)
                      .setUniform("v",           camera.matrixView())
                      .setUniform("p",           camera.matrixProj())
                      .setUniform("pc",          pc)
                      .setUniform("zNear",       camera.zNear)
                      .setUniform("zFar",        camera.zFar);

        glViewport(0, 0, renderSize.w, renderSize.h);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);
        texDepth->bindAs(GL_TEXTURE0);
        texNormal->bindAs(GL_TEXTURE1);
        rect.render();
    }
    {
        // Composite
        Binder<gl::Fbo> binder(fboComp);
        progComp.bind().setUniform("texColor",    0)
                       .setUniform("texSsr",      1)
                       .setUniform("texLight",    2)
                       .setUniform("z",           0.f)
                       .setUniform("tanHalfFov",  std::tan(0.5f * camera.fov))
                       .setUniform("aspectRatio", camera.ar)
                       .setUniform("scale",       scale);

        glViewport(0, 0, displaySize.w, displaySize.h);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        texEnv->bindAs(GL_TEXTURE0);
        texSsr.bindAs(GL_TEXTURE1);
        texLight->bindAs(GL_TEXTURE2);
        rect.render();
    }
    return *this;
}

gl::Texture* Ssr::output()
{
    return &texComp;
}

} // namespace gfx
} // namespace pt
