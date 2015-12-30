#include "grid.h"

#include "common/common.h"

namespace hc
{
namespace gfx
{

Grid::Grid() :
    grid(gridMesh(16, 128, 128)),
    vsModel(gl::Shader::path("model_pos.vs.glsl")),
    fsColor(gl::Shader::path("color.fs.glsl")),
    prog({vsModel, fsColor},
        {{0, "position"}})
{
}

Grid& Grid::operator()(gl::Fbo* fboOut, const glm::mat4& mvp)
{
    Binder<gl::Fbo> binder(fboOut);
    prog.bind()
        .setUniform("albedo", glm::vec4(0.f, 0.05f, 0.f, 1.f))
        .setUniform("mvp",    mvp);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);
    glLineWidth(2.f);

    gl::Texture::unbind(GL_TEXTURE_2D, GL_TEXTURE0);
    grid.render(GL_LINES);
    return* this;
}

} // namespace gfx
} // namespace hc
