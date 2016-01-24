#include "fader.h"

#include <glm/vec4.hpp>

namespace pt
{
namespace gfx
{

Fader::Fader() :
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsColor(gl::Shader::path("color.fs.glsl")),
    prog({vsQuad, fsColor},
        {{0, "position"}, {1, "uv"}})
{
}

Fader& Fader::operator()(float alpha)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    prog.bind().setUniform("albedo", glm::vec4(0, 0, 0, alpha));
    rect.render();
    return* this;
}

} // namespace gfx
} // namespace pt
