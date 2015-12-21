#pragma once

#include <memory>

#include <glad/glad.h>

#include "geom/rect.h"

namespace hc
{
namespace gl
{

struct Rbo
{
    Rbo();

    GLuint id() const;

    Rbo& bind();
    Rbo& unbind();

    Rbo& alloc(const Size<int>& size, GLint internalFormat);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace hc