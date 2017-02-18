#include "anti_alias.h"

#include "common/common.h"

namespace pt
{
namespace gfx
{

AntiAlias::AntiAlias(const Size<int>& renderSize) :
    renderSize(renderSize),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsFxaa(gl::Shader::path("fxaa.fs.glsl")),
    prog({vsQuad, fsFxaa},
        {{0, "position"}, {1, "uv"}})
{
    // Texture and FBO
    auto fboSize = {renderSize.w, renderSize.h};
    tex.bind().alloc(fboSize, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE)
              .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
              .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    fbo.bind()
       .attach(tex, gl::Fbo::Attachment::Color)
       .unbind();
}

AntiAlias& AntiAlias::operator()(gl::Texture* texColor)
{
    Binder<gl::Fbo> binder(fbo);
    prog.bind().setUniform("tex", 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, renderSize.w, renderSize.h);
    texColor->bindAs(GL_TEXTURE0);
    rect.render();
    return* this;
}

gl::Texture* AntiAlias::output()
{
    return &tex;
}

} // namespace gfx
} // namespace pt
