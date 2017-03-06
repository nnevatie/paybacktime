#include "ssr.h"

#include <glm/gtx/transform.hpp>

#include "platform/clock.h"
#include "common/log.h"

namespace pt
{
namespace gfx
{

Ssr::Ssr(const Size<int>& displaySize, const Size<int>& renderSize) :
    displaySize(displaySize),
    renderSize(renderSize),
    scale(float(renderSize.w) / displaySize.w),
    mipmapCount(scale * MIPMAP_COUNT_MAX),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsCommon(gl::Shader::path("common.fs.glsl")),
    fsTexture(gl::Shader::path("texture.fs.glsl")),
    fsBlur(gl::Shader::path("blur_bi.fs.glsl")),
    fsSsr(gl::Shader::path("ssr.fs.glsl")),
    fsComp(gl::Shader::path("ssr_composite.fs.glsl")),
    progScale({vsQuad, fsTexture},
              {{0, "position"}, {1, "uv"}}),
    progBlur({vsQuad, fsBlur},
            {{0, "position"}, {1, "uv"}}),
    progSsr({vsQuad, fsSsr, fsCommon},
           {{0, "position"}, {1, "uv"}}),
    progComp({vsQuad, fsComp},
            {{0, "position"}, {1, "uv"}})
{
    // Env pyramid
    auto ssrSize = {renderSize.w, renderSize.h};
    {
        std::vector<int> size = ssrSize;
        for (int i = 0; i < mipmapCount; ++i)
        {
            texScale[i].bind().alloc(size, GL_RGBA16F, GL_RGBA, GL_FLOAT)
                              .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                              .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            fboScale[i].bind()
                       .attach(texScale[i], gl::Fbo::Attachment::Color)
                       .unbind();

            texBlur[i].bind().alloc(size, GL_RGBA16F, GL_RGBA, GL_FLOAT)
                             .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                             .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            fboBlur[i].bind()
                      .attach(texBlur[i], gl::Fbo::Attachment::Color)
                      .unbind();

            texEnv.bind().alloc(i, size, GL_RGBA16F, GL_RGBA, GL_FLOAT);
            size = {size[0] / 2, size[1] / 2};
        }
        texEnv.bind()
              .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
              .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
              .set(GL_TEXTURE_MAX_LEVEL, mipmapCount - 1);
    }
    fboEnv.bind()
          .attach(texEnv, gl::Fbo::Attachment::Color)
          .unbind();

    // SSR
    texSsr.bind().alloc(ssrSize, GL_RGB16F, GL_RGB, GL_FLOAT);
    fboSsr.bind()
          .attach(texSsr, gl::Fbo::Attachment::Color)
          .unbind();

    // Composite
    auto outputSize = {displaySize.w, displaySize.h};
    texComp.bind().alloc(outputSize, GL_RGB16F, GL_RGB, GL_FLOAT);
    fboComp.bind()
           .attach(texComp, gl::Fbo::Attachment::Color)
           .unbind();
}

Ssr& Ssr::operator()(gl::Texture* texDepth,
                     gl::Texture* texNormal,
                     gl::Texture* texColor,
                     gl::Texture* texLight,
                     const Camera& camera)
{
    {
        // SSR
        Binder<gl::Fbo> binder(fboSsr);
        fboSsr.attach(texSsr, gl::Fbo::Attachment::Color);

        // View-to-screen transformation
        auto sc0 = glm::scale({}, glm::vec3(displaySize.w, displaySize.h, 1));
        auto sc1 = glm::scale({}, glm::vec3(0.5f, 0.5f, 1));
        auto tr  = glm::translate({}, glm::vec3(0.5f, 0.5f, 0));
        auto pc  = (sc0 * (tr * sc1)) * camera.matrixProj();

        progSsr.bind().setUniform("texDepth",    0)
                      .setUniform("texNormal",   1)
                      .setUniform("z",           0.f)
                      .setUniform("tanHalfFov",  std::tan(0.5f * camera.fov))
                      .setUniform("aspectRatio", camera.ar)
                      .setUniform("v",           camera.matrixView())
                      .setUniform("p",           camera.matrixProj())
                      .setUniform("pc",          pc)
                      .setUniform("zNear",       camera.zNear)
                      .setUniform("zFar",        camera.zFar);

        glViewport(0, 0, renderSize.w, renderSize.h);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);
        texDepth->bindAs(GL_TEXTURE0);
        texNormal->bindAs(GL_TEXTURE1);
        rect.render();
    }
    {
        // Env pyramid
        {
            Binder<gl::Fbo> binder(fboEnv);
            progScale.bind().setUniform("tex", 0);
            fboEnv.attach(texEnv, gl::Fbo::Attachment::Color);
            glViewport(0, 0, renderSize.w, renderSize.h);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(false);
            texColor->bindAs(GL_TEXTURE0);
            rect.render();
        }
        for (int i = 1; i < mipmapCount; ++i)
        {
            const Size<int> size(texScale[i].size().xy());
            {
                // Downscale
                Binder<gl::Fbo> binder(fboScale[i]);
                progScale.bind().setUniform("tex", 0);
                glViewport(0, 0, size.w, size.h);
                glDrawBuffer(GL_COLOR_ATTACHMENT0);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(false);
                gl::Texture scaleSrc = i > 1 ? texScale[i - 1] : texEnv;
                this->texEnv.bind().set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                scaleSrc.bindAs(GL_TEXTURE0);
                rect.render();
            }
            {
                // Blur passes
                for (int d = 0; d < 2; ++d)
                {
                    Binder<gl::Fbo> binder(d == 0 ? fboBlur[i] : fboEnv);
                    if (d == 1) fboEnv.attach(texEnv,
                                              gl::Fbo::Attachment::Color, 0, i);

                    progBlur.bind().setUniform("tex",        0)
                                   .setUniform("texDepth",   1)
                                   .setUniform("invDirSize", size.inv<glm::vec2>(d))
                                   .setUniform("radius",     3)
                                   .setUniform("sharpness",  1.f);

                    glViewport(0, 0, size.w, size.h);
                    (d == 0 ? texScale[i] : texBlur[i]).bindAs(GL_TEXTURE0);
                    texDepth->bindAs(GL_TEXTURE1);
                    rect.render();
                }
            }
        }
    }
    {
        // Composite
        Binder<gl::Fbo> binder(fboComp);
        progComp.bind().setUniform("texColor",    0)
                       .setUniform("texSsr",      1)
                       .setUniform("texLight",    2)
                       .setUniform("z",           0.f)
                       .setUniform("tanHalfFov",  std::tan(0.5f * camera.fov))
                       .setUniform("aspectRatio", camera.ar)
                       .setUniform("scale",       scale);

        glViewport(0, 0, displaySize.w, displaySize.h);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        texEnv.bindAs(GL_TEXTURE0)
              .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        texSsr.bindAs(GL_TEXTURE1);
        texLight->bindAs(GL_TEXTURE2);
        rect.render();
    }
    return *this;
}

gl::Texture* Ssr::output()
{
    return &texComp;
}

} // namespace gfx
} // namespace pt
