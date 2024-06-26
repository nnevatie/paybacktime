#include "bloom.h"

#include "common/log.h"
#include "geom/mesh.h"

namespace pt
{
namespace gfx
{

Bloom::Bloom(const Size<int>& renderSize) :
    renderSize(renderSize),
    rect(squareMesh()),
    vsQuadUv(gl::Shader::path("quad_uv.vs.glsl")),
    fsBloom(gl::Shader::path("bloom.fs.glsl")),
    fsTexture(gl::Shader::path("texture.fs.glsl")),
    fsAdd(gl::Shader::path("add.fs.glsl")),
    fsBlur(gl::Shader::path("blur.fs.glsl")),
    progBloom({vsQuadUv, fsBloom},
              {{0, "position"}, {1, "uv"}}),
    progScale({vsQuadUv, fsTexture},
              {{0, "position"}, {1, "uv"}}),
    progAdd({vsQuadUv, fsAdd},
            {{0, "position"}, {1, "uv"}}),
    progBlur({vsQuadUv, fsBlur},
            {{0, "position"}, {1, "uv"}})
{
    PTLOG(Info) << "size " << renderSize.w << "x" << renderSize.h;
    auto fboSize = {renderSize.w, renderSize.h};

    const GLenum iformat = GL_RGB16F;
    const GLenum type    = GL_FLOAT;

    // Color + emission texture and FBO
    texBloom.bind().alloc(fboSize, iformat, GL_RGB, type)
                   .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                   .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    fboBloom.bind()
            .attach(texBloom, gl::Fbo::Attachment::Color)
            .unbind();

    for (int i = 0; i < scaleCount; ++i)
    {
        const Size<int> size = renderSize / (2 << i);
        auto sizeScaled      = {size.w, size.h};

        texScale[i].bind().alloc(sizeScaled, iformat, GL_RGB, type)
                          .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                          .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        texAdd[i].bind().alloc(sizeScaled, iformat, GL_RGB, type)
                        .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                        .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        texBlur[i].bind().alloc(sizeScaled, iformat, GL_RGB, type)
                         .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                         .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        fboScale[i].bind()
                   .attach(texScale[i], gl::Fbo::Attachment::Color)
                   .unbind();
        fboAdd[i].bind()
                 .attach(texAdd[i], gl::Fbo::Attachment::Color)
                 .unbind();
        fboBlur[i].bind()
                  .attach(texBlur[i], gl::Fbo::Attachment::Color)
                  .unbind();
    }
}

Bloom& Bloom::operator()(gl::Texture* texColor)
{
    // Setup common GL states
    glDisable(GL_DEPTH_TEST);

    // Produce bloom bright map
    progBloom.bind().setUniform("texColor",  0);
    {
        Binder<gl::Fbo> binder(fboBloom);
        texColor->bindAs(GL_TEXTURE0);
        rect.render();
    }

    // Downscale
    for (int i = 0; i < scaleCount; ++i)
    {
        const auto size = texScale[i].size();
        progScale.bind().setUniform("texColor", 0);
        Binder<gl::Fbo> binder(fboScale[i]);
        glViewport(0, 0, size.x, size.y);
        gl::Texture scaleSrc = i > 0 ? texScale[i - 1] : texBloom;
        scaleSrc.bindAs(GL_TEXTURE0);
        rect.render();
    }

    // Blur and combine to next scale level up
    for (int i = scaleCount - 1; i >= 0; --i)
    {
        const Size<int> size(texScale[i].size().xy());

        // Blur source will be either the downscaled texture or it
        // combined with the previous level's blurred version.
        gl::Texture blurSrc = texScale[i];
        if (i < scaleCount - 1)
        {
            // Add previous level
            progAdd.bind().setUniform("tex0", 0).setUniform("tex1", 1);
            Binder<gl::Fbo> binder(fboAdd[i]);
            glViewport(0, 0, size.w, size.h);
            texBloom.bindAs(GL_TEXTURE0);
            texScale[i + 1].bindAs(GL_TEXTURE1);
            rect.render();
            blurSrc = texAdd[i];
        }
        // Blur passes
        for (int d = 0; d < 2; ++d)
        {
            Binder<gl::Fbo> binder(d == 0 ? fboBlur[i] : fboScale[i]);
            progBlur.bind().setUniform("tex",        0)
                           .setUniform("invDirSize", size.inv<glm::vec2>(d))
                           .setUniform("radius",     3);

            glViewport(0, 0, size.w, size.h);
            (d == 0 ? blurSrc : texBlur[i]).bindAs(GL_TEXTURE0);
            rect.render();
        }
    }

    // Restore viewport
    glViewport(0, 0, renderSize.w, renderSize.h);
    return *this;
}

gl::Texture* Bloom::output()
{
    return &texScale[0];
}

} // namespace gfx
} // namespace pt
