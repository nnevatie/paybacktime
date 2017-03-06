#include "erode.h"

#include "common/common.h"
#include "common/log.h"

#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/fbo.h"

namespace pt
{
namespace gfx
{

struct Erode::Data
{
    Data(const Size<int>& size) :
        size(size),
        rect(squareMesh()),
        vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
        fsErode(gl::Shader::path("erode.fs.glsl")),
        prog({vsQuad, fsErode}, {{0, "position"}, {1, "uv"}})
    {
        auto dim = {size.w, size.h};
        for (int i = 0; i < 2; ++i)
            texErode[i].bind().alloc(dim, GL_RGBA16F, GL_RGBA, GL_FLOAT)
                             .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                             .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    Size<int>         size;
    gl::Primitive     rect;
    gl::Fbo           fbo;
    gl::Texture       texErode[2];
    gl::Shader        vsQuad, fsErode;
    gl::ShaderProgram prog;
};

Erode::Erode(const Size<int>& size) :
    d(std::make_shared<Data>(size))
{}

Erode& Erode::operator()(gl::Texture* tex, int radius)
{
    Binder<gl::Fbo> binder(d->fbo);
    d->prog.bind().setUniform("tex",    0)
                  .setUniform("radius", radius);

    const Size<int> size(d->size);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, size.w, size.h);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);

    for (int i = 0; i < 2; ++i)
    {
        auto s = size.inv<glm::vec2>(i);
        d->fbo.attach(d->texErode[i], gl::Fbo::Attachment::Color, 0);
        d->prog.setUniform("invDirSize", size.inv<glm::vec2>(i));
        (i == 0 ? *tex : d->texErode[0]).bindAs(GL_TEXTURE0);
        d->rect.render();
    }
    return *this;
}

gl::Texture& Erode::output()
{
    return d->texErode[1];
}

} // namespace gfx
} // namespace pt
