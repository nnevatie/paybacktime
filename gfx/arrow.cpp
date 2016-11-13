#include "arrow.h"

#include <glm/gtx/transform.hpp>

#include "common/common.h"

namespace pt
{
namespace gfx
{

Arrow::Arrow() :
    arrow(triMesh(8.f)),
    vsArrow(gl::Shader::path("model_pos.vs.glsl")),
    fsArrow(gl::Shader::path("color.fs.glsl")),
    progArrow({vsArrow, fsArrow}, {{0, "position"}})
{
}

Arrow& Arrow::operator()(gl::Fbo* fboOut, const glm::mat4& mvp)
{
    Binder<gl::Fbo> binder(fboOut);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);

    const auto t = glm::translate(glm::vec3(8.f, 0.f, 0.f));
    progArrow.bind().setUniform("mvp", mvp * t)
                    .setUniform("albedo", glm::vec4(0.f, 0.5f, 0.75f, 1.f));
    arrow.render();
    return* this;
}

} // namespace gfx
} // namespace pt
