#include "ssr.h"

#include <glm/gtx/transform.hpp>

#include "common/log.h"

namespace pt
{
namespace gfx
{

Ssr::Ssr(const Size<int>& renderSize) :
    renderSize(renderSize),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsCommon(gl::Shader::path("common.fs.glsl")),
    fsSsr(gl::Shader::path("ssr.fs.glsl")),
    prog({vsQuad, fsSsr, fsCommon},
        {{0, "position"}, {1, "uv"}})
{
    // Texture and FBO
    auto fboSize = {renderSize.w, renderSize.h};
    {
        Binder<gl::Texture> binder(texColor);
        std::vector<int> size = fboSize;
        for (int i = 0; i < mipmapCount; ++i)
        {
            texColor.alloc(i, size, GL_RGB32F,  GL_RGB, GL_FLOAT);
            size = {size[0] / 2, size[1] / 2};
        }
        texColor.set(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
                .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
                .set(GL_TEXTURE_MAX_LEVEL, mipmapCount - 1);
    }
    texSsr.bind().alloc(fboSize, GL_RGB32F, GL_RGB, GL_FLOAT);
    fbo.bind()
       .attach(texSsr, gl::Fbo::Attachment::Color)
       .unbind();
}

Ssr& Ssr::operator()(gl::Texture* texDepth,
                     gl::Texture* texDepthBack,
                     gl::Texture* texNormal,
                     gl::Texture* texColor,
                     gl::Texture* texLight,
                     const Camera& camera)
{
    // View-to-screen transformation
    auto sc0 = glm::scale({}, glm::vec3(renderSize.w, renderSize.h, 1));
    auto sc1 = glm::scale({}, glm::vec3(0.5f, 0.5f, 1));
    auto tr  = glm::translate({}, glm::vec3(0.5f, 0.5f, 0));
    auto pc  = (sc0 * (tr * sc1)) * camera.matrixProj();

    Binder<gl::Fbo> binder(fbo);
    prog.bind().setUniform("texDepth",    0)
               .setUniform("texDepthBack",1)
               .setUniform("texNormal",   2)
               .setUniform("texColor",    3)
               .setUniform("texLight",    4)
               .setUniform("z",           0.f)
               .setUniform("tanHalfFov",  std::tan(0.5f * camera.fov))
               .setUniform("aspectRatio", camera.ar)
               .setUniform("viewPos",     camera.position())
               .setUniform("v",           camera.matrixView())
               .setUniform("p",           camera.matrixProj())
               .setUniform("pc",          pc)
               .setUniform("zNear",       camera.zNear)
               .setUniform("zFar",        camera.zFar);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glDisable(GL_DEPTH_TEST);
    texDepth->bindAs(GL_TEXTURE0);
    texDepthBack->bindAs(GL_TEXTURE1);
    texNormal->bindAs(GL_TEXTURE2);
    texColor->bindAs(GL_TEXTURE3);
    texLight->bindAs(GL_TEXTURE4);
    rect.render();
    return *this;
}

gl::Texture* Ssr::output()
{
    return &texSsr;
}

} // namespace gfx
} // namespace pt
