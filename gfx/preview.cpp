#include "preview.h"

#include "common/common.h"

namespace pt
{
namespace gfx
{

Preview::Preview(const Size<int>& renderSize) :
    renderSize(renderSize),
    rect(squareMesh()),
    vsModel(gl::Shader::path("model.vs.glsl")),
    fsModel(gl::Shader::path("model.fs.glsl")),
    prog({vsModel, fsModel},
        {{0, "position"}, {1, "normal"}, {2, "uv"}})
{
    auto fboSize = {renderSize.w, renderSize.h};
    texDepth.bind().alloc(fboSize, GL_DEPTH_COMPONENT32F,
                                   GL_DEPTH_COMPONENT, GL_FLOAT);
    texColor.bind().alloc(fboSize, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE)
                   .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                   .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    fbo.bind()
       .attach(texDepth, gl::Fbo::Attachment::Depth)
       .attach(texColor, gl::Fbo::Attachment::Color)
       .unbind();
}

Preview& Preview::operator()(
    gl::Texture* texAlbedo,
    const gl::Primitive& primitive,
    const glm::mat4& mvp)
{
    Binder<gl::Fbo> binder(fbo);
    prog.bind()
        .setUniform("texAlbedo", 0)
        .setUniform("mvp",       mvp);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);
    glDepthMask(true);

    glViewport(0, 0, renderSize.w, renderSize.h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    texAlbedo->bindAs(GL_TEXTURE0);
    primitive.render();
    return *this;
}

} // namespace gfx
} // namespace pt
