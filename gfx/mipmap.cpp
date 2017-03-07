#include "mipmap.h"

#include "common/common.h"
#include "common/log.h"

#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/fbo.h"

#include "blur.h"

namespace pt
{
namespace gfx
{
using TexPyramid  = std::vector<gl::Texture>;
using FboPyramid  = std::vector<gl::Fbo>;
using BlurPyramid = std::vector<Blur>;

struct Mipmap::Data
{
    Data(const Size<int>& size, int depth, bool bilateral) :
        size(size),
        rect(squareMesh()),
        vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
        fsTex(gl::Shader::path("texture.fs.glsl")),
        prog({vsQuad, fsTex}, {{0, "position"}, {1, "uv"}})
    {
        // TODO: Configurable pixel type
        std::vector<int> dim = {size.w, size.h};
        for (int i = 0; i < depth; ++i)
        {
            gl::Texture texScale;
            texScale.bind().alloc(dim, GL_RGBA16F, GL_RGBA, GL_FLOAT)
                           .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                           .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            texScales.push_back(texScale);

            gl::Fbo fboScale;
            fboScale.bind()
                    .attach(texScale, gl::Fbo::Attachment::Color)
                    .unbind();
            fboScales.push_back(fboScale);

            tex.bind().alloc(i, dim, GL_RGBA16F, GL_RGBA, GL_FLOAT).unbind();
            blurScales.push_back(Blur({dim[0], dim[1]}, &tex, i, bilateral));

            dim = {dim[0] / 2, dim[1] / 2};
        }
        tex.bind()
           .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
           .set(GL_TEXTURE_MAX_LEVEL, depth - 1);

        fbo.bind().attach(tex, gl::Fbo::Attachment::Color).unbind();
    }

    Size<int>         size;
    gl::Primitive     rect;
    gl::Fbo           fbo;
    gl::Texture       tex;
    TexPyramid        texScales;
    FboPyramid        fboScales;
    BlurPyramid       blurScales;
    gl::Shader        vsQuad, fsTex;
    gl::ShaderProgram prog;
};

Mipmap::Mipmap(const Size<int>& size, int depth, bool bilateral) :
    d(std::make_shared<Data>(size, depth, bilateral))
{}

Mipmap& Mipmap::operator()(gl::Texture* tex, gl::Texture* texDepth)
{
    {
        // Top level
        Binder<gl::Fbo> binder(d->fbo);
        d->fbo.attach(d->tex, gl::Fbo::Attachment::Color);
        d->prog.bind().setUniform("tex", 0);
        glViewport(0, 0, d->size.w, d->size.h);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);
        tex->bindAs(GL_TEXTURE0);
        d->rect.render();
    }
    // Downscale
    d->tex.bind().set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    for (int i = 1; i < int(d->fboScales.size()); ++i)
    {
        const Size<int> size(d->texScales[i].size().xy());
        Binder<gl::Fbo> binder(d->fboScales[i]);
        d->prog.bind().setUniform("tex", 0);
        glViewport(0, 0, size.w, size.h);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);
        gl::Texture& scaleSrc = i > 1 ? d->texScales[i - 1] : d->tex;
        scaleSrc.bindAs(GL_TEXTURE0);
        d->rect.render();
        d->blurScales[i](&d->texScales[i], texDepth, 3, 25.f);
    }
    d->tex.bindAs(GL_TEXTURE0).set(GL_TEXTURE_MIN_FILTER,
                                   GL_LINEAR_MIPMAP_LINEAR);
    return *this;
}

gl::Texture* Mipmap::output()
{
    return &d->tex;
}

} // namespace gfx
} // namespace pt
