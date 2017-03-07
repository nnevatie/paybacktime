#include "color_grade.h"

#include "common/common.h"

namespace pt
{
namespace gfx
{

ColorGrade::ColorGrade(const Size<int>& renderSize) :
    renderSize(renderSize),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsColorGrade(gl::Shader::path("color_grade.fs.glsl")),
    prog({vsQuad, fsColorGrade},
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

ColorGrade& ColorGrade::operator()(gl::Texture* texColor, gl::Texture* texBloom)
{
    Binder<gl::Fbo> binder(fbo);
    prog.bind().setUniform("tex0", 0)
               .setUniform("tex1", 1);
    glViewport(0, 0, renderSize.w, renderSize.h);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glDisable(GL_DEPTH_TEST);
    texColor->bindAs(GL_TEXTURE0);
    texBloom->bindAs(GL_TEXTURE1);
    rect.render();
    return *this;
}

gl::Texture* ColorGrade::output()
{
    return &tex;
}

} // namespace gfx
} // namespace pt
