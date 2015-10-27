#pragma once

#include <memory>

#include <GL/glew.h>

#include "geometry.h"

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
