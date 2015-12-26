#include "geometry.h"

#include "common/common.h"

namespace hc
{
namespace gfx
{

Geometry::Geometry(const Size<int>& renderSize) :
    renderSize(renderSize),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    vsGeometry(gl::Shader::path("geometry.vs.glsl")),
    gsWireframe(gl::Shader::path("wireframe.gs.glsl")),
    fsGeometry(gl::Shader::path("geometry.fs.glsl")),
    fsDenoise(gl::Shader::path("denoise.fs.glsl")),
    fsCommon(gl::Shader::path("common.fs.glsl")),

    progGeometry({vsGeometry, gsWireframe, fsGeometry, fsCommon},
                {{0, "position"}, {1, "normal"}, {2, "uv"}}),
    progDenoise({vsQuad, fsDenoise},
        {{0, "position"}, {1, "uv"}})
{
    // Texture and FBO
    auto fboSize = {renderSize.w, renderSize.h};

    texDepth.bind().alloc(fboSize,         GL_DEPTH_COMPONENT32F,
                                           GL_DEPTH_COMPONENT, GL_FLOAT);
    texNormal.bind().alloc(fboSize,        GL_RGB16F,  GL_RGB, GL_FLOAT);
    texNormalDenoise.bind().alloc(fboSize, GL_RGB16F,  GL_RGB, GL_FLOAT);
    texColor.bind().alloc(fboSize,         GL_RGB8,    GL_RGB, GL_UNSIGNED_BYTE);
    texLight.bind().alloc(fboSize,         GL_RGB8,    GL_RGB, GL_UNSIGNED_BYTE);

    fbo.bind()
       .attach(texDepth,         gl::Fbo::Attachment::Depth)
       .attach(texNormal,        gl::Fbo::Attachment::Color, 0)
       .attach(texColor,         gl::Fbo::Attachment::Color, 1)
       .attach(texLight,         gl::Fbo::Attachment::Color, 2)
       .attach(texNormalDenoise, gl::Fbo::Attachment::Color, 3)
       .unbind();
}

Geometry& Geometry::operator()(
    gl::Texture* texAlbedo,
    gl::Texture* texLightmap,
    const std::vector<Instance>& instances,
    const glm::mat4& v,
    const glm::mat4& p)
{
    Binder<gl::Fbo> binder(fbo);
    progGeometry.bind()
                .setUniform("texAlbedo", 0)
                .setUniform("texLight",  1)
                .setUniform("v",         v)
                .setUniform("p",         p)
                .setUniform("size",      renderSize.as<glm::vec2>());

    const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                                  GL_COLOR_ATTACHMENT1,
                                  GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, drawBuffers);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);
    glDepthMask(true);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    texAlbedo->bindAs(GL_TEXTURE0);
    texLightmap->bindAs(GL_TEXTURE1);

    // Render primitives
    for (const auto& instance : instances)
    {
        progGeometry.setUniform("m", instance.second);
        instance.first.render();
    }

    // Denoise normals
    progDenoise.bind().setUniform("tex", 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT3);
    glDisable(GL_DEPTH_TEST);
    texNormal.bindAs(GL_TEXTURE0);
    rect.render();
    return *this;
}

} // namespace gfx
} // namespace hc
