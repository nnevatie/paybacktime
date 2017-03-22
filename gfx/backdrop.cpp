#include "backdrop.h"

#include "common/common.h"

namespace pt
{
namespace gfx
{

Backdrop::Backdrop(const Size<int>& renderSize) :
    renderSize(renderSize),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsBackdrop(gl::Shader::path("backdrop.fs.glsl")),
    prog({vsQuad, fsBackdrop},
        {{0, "position"}, {1, "uv"}})
{
    tex.bind().alloc(Image("data/backdrop.png"))
              .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
              .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Backdrop& Backdrop::operator()(gl::Fbo* fboOut, const Camera& camera)
{
    Binder<gl::Fbo> binder(fboOut);
    prog.bind()
        .setUniform("tex",         0)
        .setUniform("v",           camera.matrixView())
        .setUniform("pos",         camera.position())
        .setUniform("z",           1.f)
        .setUniform("tanHalfFov",  camera.tanHalfFov())
        .setUniform("aspectRatio", camera.ar)
        .setUniform("gridColor",   glm::vec4(0, 0.5, 0, 1));

    glViewport(0, 0, renderSize.w, renderSize.h);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    tex.bindAs(GL_TEXTURE0);
    rect.render();
    return* this;
}

} // namespace gfx
} // namespace pt
