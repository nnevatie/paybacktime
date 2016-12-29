#pragma once

#include <memory>

#include "platform/gl.h"
#include "geom/size.h"

namespace pt
{
namespace gl
{

struct Rbo
{
    Rbo();

    GLuint id() const;

    Rbo& bind();
    Rbo& unbind();

    Rbo& alloc(const Size<int>& size, GLenum internalFormat);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace pt
