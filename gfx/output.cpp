#include "output.h"

namespace pt
{
namespace gfx
{

Output::Output() :
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsTex(gl::Shader::path("texture.fs.glsl")),
    prog({vsQuad, fsTex},
        {{0, "position"}, {1, "uv"}})
{
}

Output& Output::operator()(gl::Texture* tex)
{
    prog.bind().setUniform("tex", 0);
    glDisable(GL_DEPTH_TEST);
    tex->bindAs(GL_TEXTURE0);
    rect.render();
    return* this;
}

} // namespace gfx
} // namespace pt
