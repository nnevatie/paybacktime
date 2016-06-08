#include "geometry.h"

#include "common/common.h"

namespace pt
{
namespace gfx
{

Geometry::Geometry(const Size<int>& renderSize) :
    renderSize(renderSize),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    vsGeometry(gl::Shader::path("geometry.vs.glsl")),
    vsGeometryTransparent(gl::Shader::path("geometry_transparent.vs.glsl")),
    gsWireframe(gl::Shader::path("wireframe.gs.glsl")),
    fsGeometry(gl::Shader::path("geometry.fs.glsl")),
    fsGeometryTransparent(gl::Shader::path("geometry_transparent.fs.glsl")),
    fsOitComposite(gl::Shader::path("oit_composite.fs.glsl")),
    fsDenoise(gl::Shader::path("denoise.fs.glsl")),
    fsCommon(gl::Shader::path("common.fs.glsl")),
    progGeometry({vsGeometry, /*gsWireframe,*/ fsGeometry, fsCommon},
                {{0, "position"}, {1, "normal"}, {2, "tangent"}, {3, "uv"}}),
    progGeometryTransparent({vsGeometryTransparent, fsGeometryTransparent, fsCommon},
                {{0, "position"}, {1, "normal"}, {2, "tangent"}, {3, "uv"}}),
    progOitComposite({vsQuad, fsOitComposite},
                    {{0, "position"}, {1, "uv"}}),
    progDenoise({vsQuad, fsDenoise},
        {{0, "position"}, {1, "uv"}})
{
    auto fboSize = {renderSize.w, renderSize.h};

    texDepth.bind().alloc(fboSize, GL_DEPTH_COMPONENT32F,
                                   GL_DEPTH_COMPONENT, GL_FLOAT)
                   .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                   .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

    // OIT
    texOit0.bind().alloc(fboSize, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    texOit1.bind().alloc(fboSize, GL_R16F,    GL_RED,  GL_FLOAT);
    fboOit.bind()
          .attach(texDepth, gl::Fbo::Attachment::Depth)
          .attach(texOit0,  gl::Fbo::Attachment::Color, 0)
          .attach(texOit1,  gl::Fbo::Attachment::Color, 1)
          .unbind();
}

Geometry& Geometry::operator()(
    gl::Texture* texAlbedo,
    gl::Texture* texNormalMap,
    gl::Texture* texLightmap,
    const Instances& instances,
    const glm::mat4& v,
    const glm::mat4& p)
{
    {
        // Front faces
        Binder<gl::Fbo> binder(fbo);
        progGeometry.bind()
                    .setUniform("texAlbedo", 0)
                    .setUniform("texNormal", 1)
                    .setUniform("texLight",  2)
                    .setUniform("v",         v)
                    .setUniform("p",         p)
                    .setUniform("size",      renderSize.as<glm::vec2>());

        const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                                      GL_COLOR_ATTACHMENT1,
                                      GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, drawBuffers);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDisable(GL_BLEND);
        glDepthMask(true);

        glViewport(0, 0, renderSize.w, renderSize.h);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        texAlbedo->bindAs(GL_TEXTURE0);
        texNormalMap->bindAs(GL_TEXTURE1);
        texLightmap->bindAs(GL_TEXTURE2);

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
    }
    return *this;
}

Geometry& Geometry::operator()(
    gl::Fbo* fbo,
    gl::Texture* texAlbedo,
    gl::Texture* texLightmap,
    gl::Texture* texGi,
    gl::Texture* texIncid,
    const Box& bounds,
    const Instances& instances,
    const Camera& camera)
{
    // OIT accumulation passes
    {
        Binder<gl::Fbo> binder(fboOit);
        progGeometryTransparent.bind()
                               .setUniform("texAlbedo",  0)
                               .setUniform("texLight",   1)
                               .setUniform("texGi",      2)
                               .setUniform("texIncid",   3)
                               .setUniform("boundsMin",  glm::floor(bounds.pos))
                               .setUniform("boundsSize", glm::ceil(bounds.size))
                               .setUniform("viewPos",    camera.position())
                               .setUniform("v",          camera.matrixView())
                               .setUniform("p",          camera.matrixProj());
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(false);
        glEnable(GL_BLEND);

        texAlbedo->bindAs(GL_TEXTURE0);
        texLightmap->bindAs(GL_TEXTURE1);
        texGi->bindAs(GL_TEXTURE2);
        texIncid->bindAs(GL_TEXTURE3);

        // Active buffer
        const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                                      GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, drawBuffers);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Blend functions
        glBlendFuncSeparate(GL_ONE,  GL_ONE,
                            GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

        // Render primitives
        for (const auto& instance : instances)
        {
            progGeometryTransparent.setUniform("m", instance.second);
            instance.first.render();
        }
        glDisable(GL_BLEND);
    }
    // OIT composition pass
    {
        Binder<gl::Fbo> binder(*fbo);
        progOitComposite.bind()
                        .setUniform("tex0", 0)
                        .setUniform("tex1", 1);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
        texOit0.bindAs(GL_TEXTURE0);
        texOit1.bindAs(GL_TEXTURE1);
        rect.render();
        glDisable(GL_BLEND);
    }
    return *this;
}

} // namespace gfx
} // namespace pt
