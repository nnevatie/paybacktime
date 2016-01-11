#include "grid.h"

#include "common/common.h"

namespace pt
{
namespace gfx
{

Grid::Grid() :
    rect(squareMesh()),
    vsModel(gl::Shader::path("quad_uv.vs.glsl")),
    fsColor(gl::Shader::path("grid.fs.glsl")),
    prog({vsModel, fsColor},
        {{0, "position"}, {1, "uv"}})
{
}

Grid& Grid::operator()(gl::Fbo* fboOut, const Camera& camera)
{
    Binder<gl::Fbo> binder(fboOut);
    prog.bind()
        .setUniform("v",           camera.matrixView())
        .setUniform("pos",         camera.position())
        .setUniform("z",           1.f)
        .setUniform("tanHalfFov",  std::tan(0.5f * camera.fov))
        .setUniform("aspectRatio", camera.ar)
        .setUniform("gridColor",   glm::vec4(0, 0.5, 0, 1));

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(false);

    rect.render();
    return* this;
}

} // namespace gfx
} // namespace pt
