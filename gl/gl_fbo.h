#pragma once

#include <memory>

#include "gl_rbo.h"
#include "gl_texture.h"

namespace hc
{
namespace gl
{

struct Fbo
{
    enum class Attachment
    {
        Color,
        Depth,
        Stencil,
        DepthStencil
    };

    Fbo();

    Fbo& bind();

    Fbo& attach(const Rbo& rbo, Attachment attachment, int index = 0);
    Fbo& attach(const Texture& texture, Attachment attachment, int index = 0);

    static bool unbind();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace hc
