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
    fsGaussian(gl::Shader::path("gaussian.fs.glsl")),
    fsSsr(gl::Shader::path("ssr.fs.glsl")),
    fsSsrComposite(gl::Shader::path("ssr_composite.fs.glsl")),
    progScale({vsQuad, fsTexture},
              {{0, "position"}, {1, "uv"}}),
    progBlur({vsQuad, fsGaussian},
            {{0, "position"}, {1, "uv"}}),
    progSsr({vsQuad, fsSsr, fsCommon},
           {{0, "position"}, {1, "uv"}}),
    progSsrComposite({vsQuad, fsSsrComposite},
                    {{0, "position"}, {1, "uv"}})
{
    // Texture and FBO
    auto fboSize = {renderSize.w, renderSize.h};
    {
        std::vector<int> size = fboSize;
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

            texColor.bind().alloc(i, size, GL_RGBA16F, GL_RGBA, GL_FLOAT);
            size = {size[0] / 2, size[1] / 2};
        }
        texColor.bind()
                .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
                .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
                .set(GL_TEXTURE_MAX_LEVEL, mipmapCount - 1);
    }
    fboColor.bind()
            .attach(texColor, gl::Fbo::Attachment::Color)
            .unbind();

    auto outputSize = {displaySize.w, displaySize.h};
    texSsr.bind().alloc(outputSize, GL_RGB16F, GL_RGB, GL_FLOAT);
    fboSsr.bind()
          .attach(texSsr, gl::Fbo::Attachment::Color)
          .unbind();
}

Ssr& Ssr::operator()(gl::Texture* texDepth,
                     gl::Texture* texNormal,
                     gl::Texture* texColor,
                     gl::Texture* texLight,
                     const Camera& camera)
{
    {
        Binder<gl::Fbo> binder(fboColor);
        fboColor.attach(this->texColor, gl::Fbo::Attachment::Color);

        // View-to-screen transformation
        auto sc0 = glm::scale({}, glm::vec3(displaySize.w, displaySize.h, 1));
        auto sc1 = glm::scale({}, glm::vec3(0.5f, 0.5f, 1));
        auto tr  = glm::translate({}, glm::vec3(0.5f, 0.5f, 0));
        auto pc  = (sc0 * (tr * sc1)) * camera.matrixProj();

        progSsr.bind().setUniform("texDepth",    0)
                      .setUniform("texNormal",   1)
                      .setUniform("texColor",    2)
                      .setUniform("texLight",    3)
                      .setUniform("z",           0.f)
                      .setUniform("tanHalfFov",  std::tan(0.5f * camera.fov))
                      .setUniform("aspectRatio", camera.ar)
                      .setUniform("viewPos",     camera.position())
                      .setUniform("v",           camera.matrixView())
                      .setUniform("p",           camera.matrixProj())
                      .setUniform("pc",          pc)
                      .setUniform("zNear",       camera.zNear)
                      .setUniform("zFar",        camera.zFar)
                      .setUniform("scale",       scale);

        glViewport(0, 0, renderSize.w, renderSize.h);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);
        texDepth->bindAs(GL_TEXTURE0);
        texNormal->bindAs(GL_TEXTURE1);
        texColor->bindAs(GL_TEXTURE2);
        texLight->bindAs(GL_TEXTURE3);
        rect.render();
    }
    {
        // Mipmap levels
        for (int i = 1; i < mipmapCount; ++i)
        {
            const auto size = texScale[i].size();
            {
                // Downscale
                Binder<gl::Fbo> binder(fboScale[i]);
                progScale.bind().setUniform("texColor", 0);

                glViewport(0, 0, size.x, size.y);
                glDrawBuffer(GL_COLOR_ATTACHMENT0);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(false);
                gl::Texture scaleSrc = i > 1 ? texScale[i - 1] : this->texColor;
                this->texColor.bind().set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                scaleSrc.bindAs(GL_TEXTURE0);
                rect.render();
            }
            {
                // Blur
                progBlur.bind().setUniform("texColor", 0);
                {
                    // Horizontal
                    progBlur.setUniform("horizontal", true);
                    Binder<gl::Fbo> binder(fboBlur[i]);
                    glViewport(0, 0, size.x, size.y);
                    texScale[i].bindAs(GL_TEXTURE0);
                    rect.render();
                }
                {
                    // Vertical
                    progBlur.setUniform("horizontal", false);
                    Binder<gl::Fbo> binder(fboScale[i]);
                    glViewport(0, 0, size.x, size.y);
                    texBlur[i].bindAs(GL_TEXTURE0);
                    rect.render();
                }
            }
            {
                // Copy to mipmap
                Binder<gl::Fbo> binder(fboColor);
                progScale.bind().setUniform("texColor", 0);
                fboColor.attach(this->texColor, gl::Fbo::Attachment::Color, 0, i);
                glViewport(0, 0, size.x, size.y);
                glDrawBuffer(GL_COLOR_ATTACHMENT0);
                texScale[i].bindAs(GL_TEXTURE0);
                rect.render();
            }
        }
    }
    {
        // Composite
        Binder<gl::Fbo> binder(fboSsr);
        progSsrComposite.bind().setUniform("texColor",    0)
                               .setUniform("texSsr",      1)
                               .setUniform("texLight",    2)
                               .setUniform("z",           0.f)
                               .setUniform("tanHalfFov",  std::tan(0.5f * camera.fov))
                               .setUniform("aspectRatio", camera.ar)
                               .setUniform("scale",       scale);

        glViewport(0, 0, displaySize.w, displaySize.h);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        this->texColor.bind().set(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        texColor->bindAs(GL_TEXTURE0);
        this->texColor.bindAs(GL_TEXTURE1);
        texLight->bindAs(GL_TEXTURE2);
        rect.render();
    }
    return *this;
}

gl::Texture* Ssr::output()
{
    return &texSsr;
}

} // namespace gfx
} // namespace pt
