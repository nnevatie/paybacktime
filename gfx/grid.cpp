#include "grid.h"

#include "common/common.h"

namespace pt
{
namespace gfx
{

Grid::Grid() :
    grid(gridMesh(16, 128, 128)),
    rect(squareMesh()),
    vsModel(gl::Shader::path("quad_uv.vs.glsl")),
    fsColor(gl::Shader::path("grid.fs.glsl")),
    prog({vsModel, fsColor},
        {{0, "position"}, {1, "uv"}})
{
}

Grid& Grid::operator()(gl::Fbo* fboOut, gl::Texture* texDepth,
                       const Camera& camera)
{
    Binder<gl::Fbo> binder(fboOut);
    prog.bind()
        .setUniform("texDepth",    0)
        .setUniform("pos",         camera.position())
        .setUniform("yaw",         camera.yaw)
        .setUniform("pitch",       camera.pitch)
        .setUniform("tanHalfFov",  std::tan(0.5f * camera.fov))
        .setUniform("aspectRatio", camera.ar)
        .setUniform("albedo",      glm::vec4(0, 0.5, 0, 1));

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);

    texDepth->bindAs(GL_TEXTURE0);
    rect.render();
    return* this;
}

} // namespace gfx
} // namespace pt
