#include "outline.h"

#include <glbinding/gl/bitfield.h>

#include "common/common.h"

namespace pt
{
namespace gfx
{

Outline::Outline(const Size<int>& renderSize, const gl::Texture& texDepth) :
    renderSize(renderSize),
    rect(squareMesh()),
    vsModel(gl::Shader::path("model_pos.vs.glsl")),
    fsModel(gl::Shader::path("color.fs.glsl")),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsDenoise(gl::Shader::path("denoise.fs.glsl")),
    fsOutline(gl::Shader::path("outline.fs.glsl")),
    progModel({vsModel, fsModel},
              {{0, "position"}, {1, "normal"}, {2, "uv"}}),
    progDenoise({vsQuad, fsDenoise},
                {{0, "position"}, {1, "uv"}}),
    progOutline({vsQuad, fsOutline},
                {{0, "position"}, {1, "uv"}})
{
    // Model texture and FBO
    auto fboSize = {renderSize.w, renderSize.h};
    texModel.bind().alloc(fboSize, GL_R8, GL_RGB, GL_UNSIGNED_BYTE);
    texDenoise.bind().alloc(fboSize, GL_R8, GL_RGB, GL_UNSIGNED_BYTE);
    fboModel.bind()
            .attach(texDepth, gl::Fbo::Attachment::Depth)
            .attach(texModel, gl::Fbo::Attachment::Color)
            .unbind();
    fboDenoise.bind()
            .attach(texDepth, gl::Fbo::Attachment::Depth)
            .attach(texDenoise, gl::Fbo::Attachment::Color)
            .unbind();
}

Outline& Outline::operator()(gl::Fbo* fboOut,
                             gl::Texture* texColor,
                             const gl::Primitive& primitive,
                             const glm::mat4& mvp,
                             const glm::vec4& color)
{
    {
        Binder<gl::Fbo> binder(fboModel);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(false);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0, -1000.f);

        glClear(GL_COLOR_BUFFER_BIT);
        gl::Texture::unbind(GL_TEXTURE_2D, GL_TEXTURE0);

        progModel.bind()
            .setUniform("mvp", mvp)
            .setUniform("albedo", glm::vec4(1, 1, 1, 1));
        primitive.render();
    }
    {
        Binder<gl::Fbo> binder(fboDenoise);
        progDenoise.bind().setUniform("tex", 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        texModel.bindAs(GL_TEXTURE0);
        rect.render();
    }
    {
        Binder<gl::Fbo> binder(fboOut);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);

        texColor->bindAs(GL_TEXTURE0);
        texDenoise.bindAs(GL_TEXTURE1);
        progOutline.bind().setUniform("tex", 0)
                          .setUniform("outline", 1)
                          .setUniform("fillColor", color);
        rect.render();
    }
    return* this;
}

} // namespace gfx
} // namespace pt
