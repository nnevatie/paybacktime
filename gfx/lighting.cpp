#include "lighting.h"

#include "common/common.h"

namespace pt
{
namespace gfx
{

Lighting::Lighting(const Size<int>& renderSize, const gl::Texture& texDepth) :
    renderSize(renderSize),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsLighting(gl::Shader::path("lighting.fs.glsl")),
    fsCommon(gl::Shader::path("common.fs.glsl")),
    prog({vsQuad, fsLighting, fsCommon},
        {{0, "position"}, {1, "uv"}})
{
    // Texture and FBO
    auto fboSize = {renderSize.w, renderSize.h};
    tex.bind().alloc(fboSize, GL_RGB16F,  GL_RGB, GL_FLOAT);
    fbo.bind()
       .attach(texDepth, gl::Fbo::Attachment::Depth)
       .attach(tex, gl::Fbo::Attachment::Color)
       .unbind();
}

Lighting& Lighting::operator()(
    gl::Texture* texDepth,
    gl::Texture* texNormal,
    gl::Texture* texColor,
    gl::Texture* texLight,
    gl::Texture* texBloom,
    gl::Texture* texSsao,
    gl::Texture* texLightmap,
    const glm::mat4& v,
    const glm::mat4& p)
{
    Binder<gl::Fbo> binder(fbo);
    prog.bind().setUniform("texDepth",    0)
               .setUniform("texNormal",   1)
               .setUniform("texColor",    2)
               .setUniform("texLight",    3)
               .setUniform("texAo",       4)
               .setUniform("texGi",       5)
               .setUniform("v",           v)
               .setUniform("p",           p);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    glClear(GL_DEPTH_BUFFER_BIT);

    texDepth->bindAs(GL_TEXTURE0);
    texNormal->bindAs(GL_TEXTURE1);
    texColor->bindAs(GL_TEXTURE2);
    texLight->bindAs(GL_TEXTURE3);
    texSsao->bindAs(GL_TEXTURE4);
    texLightmap->bindAs(GL_TEXTURE5);
    rect.render();
    return *this;
}

gl::Texture* Lighting::output()
{
    return &tex;
}

} // namespace gfx
} // namespace pt
