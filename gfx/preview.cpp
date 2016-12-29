#include "preview.h"

#include <glbinding/gl/bitfield.h>

#include "common/common.h"

namespace pt
{
namespace gfx
{

Preview::Preview(const Size<int>& renderSize) :
    renderSize(renderSize),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    vsModel(gl::Shader::path("model.vs.glsl")),
    fsModel(gl::Shader::path("model.fs.glsl")),
    fsDenoise(gl::Shader::path("denoise.fs.glsl")),
    progModel({vsModel, fsModel},
             {{0, "position"}, {1, "normal"}, {2, "tangent"}, {3, "uv"}}),
    progDenoise({vsQuad, fsDenoise},
               {{0, "position"}, {1, "uv"}}),
    antiAlias(renderSize)
{
    auto fboSize = {renderSize.w, renderSize.h};
    texDepth.bind().alloc(fboSize, GL_DEPTH_COMPONENT32F,
                                   GL_DEPTH_COMPONENT, GL_FLOAT);
    texColor.bind().alloc(fboSize, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE)
                   .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                   .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texDenoise.bind().alloc(fboSize, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE)
                     .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                     .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    fboModel.bind()
       .attach(texDepth, gl::Fbo::Attachment::Depth)
       .attach(texColor, gl::Fbo::Attachment::Color)
       .unbind();
    fboDenoise.bind()
            .attach(texDenoise, gl::Fbo::Attachment::Color)
            .unbind();
}

Preview& Preview::operator()(
    gl::Texture* texAlbedo,
    const gl::Primitive& primitive,
    const glm::mat4& mvp)
{
    {
        Binder<gl::Fbo> binder(fboModel);
        progModel.bind()
            .setUniform("texAlbedo", 0)
            .setUniform("mvp",       mvp);

        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDisable(GL_BLEND);
        glDepthMask(true);

        glViewport(0, 0, renderSize.w, renderSize.h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        texAlbedo->bindAs(GL_TEXTURE0);
        primitive.render();
    }
    {
        Binder<gl::Fbo> binder(fboDenoise);
        progDenoise.bind().setUniform("tex", 0)
                          .setUniform("e",   0.1f);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        texColor.bindAs(GL_TEXTURE0);
        rect.render();
    }
    {
        antiAlias(&texDenoise);
    }
    return *this;
}

gl::Texture* Preview::output()
{
    return antiAlias.output();
}

} // namespace gfx
} // namespace pt
