#pragma once

#include <memory>

#include "rbo.h"
#include "texture.h"

namespace pt
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

    Fbo& attach(const Rbo& rbo, Attachment attachment,
                int index = 0);
    Fbo& attach(const Texture& texture, Attachment attachment,
                int index = 0, int level = 0, int layer = 0);

    static bool unbind();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace pt
