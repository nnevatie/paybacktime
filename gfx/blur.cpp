#include "blur.h"

#include "common/common.h"
#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/fbo.h"

namespace pt
{
namespace gfx
{

struct Blur::Data
{
    Data(const Size<int>& size, bool bilateral) :
        size(size),
        rect(squareMesh()),
        vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
        fsBlur(gl::Shader::path(bilateral ? "blur_bi.fs.glsl" : "blur.fs.glsl")),
        prog({vsQuad, fsBlur}, {{0, "position"}, {1, "uv"}})
    {
        auto dim = {size.w, size.h};
        for (int i = 0; i < 2; ++i)
            texBlur[i].bind().alloc(dim, GL_RGBA16F, GL_RGBA, GL_FLOAT)
                             .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                             .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    Size<int>         size;
    gl::Primitive     rect;
    gl::Fbo           fbo;
    gl::Texture       texBlur[2];
    gl::Shader        vsQuad, fsBlur;
    gl::ShaderProgram prog;
};

Blur::Blur(const Size<int>& size, bool bilateral) :
    d(std::make_shared<Data>(size, bilateral))
{}

Blur& Blur::operator()(gl::Texture* tex, gl::Texture* texDepth,
                       int radius, float sharpness)
{
    Binder<gl::Fbo> binder(d->fbo);
    d->prog.bind().setUniform("texColor",   0)
                  .setUniform("texDepth",   1)
                  .setUniform("radius",     radius)
                  .setUniform("sharpness",  sharpness);

    const Size<int> size(d->size);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, size.w, size.h);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);

    for (int i = 0; i < 2; ++i)
    {
        d->fbo.attach(d->texBlur[i], gl::Fbo::Attachment::Color, 0);
        d->prog.setUniform("invDirSize", size.inv<glm::vec2>(i));
        (i == 0 ? *tex : d->texBlur[0]).bindAs(GL_TEXTURE0);
        if (texDepth) texDepth->bindAs(GL_TEXTURE1);
        d->rect.render();
    }
    return *this;
}

gl::Texture& Blur::output()
{
    return d->texBlur[1];
}

} // namespace gfx
} // namespace pt
