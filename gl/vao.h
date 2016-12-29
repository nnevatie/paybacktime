#pragma once

#include <memory>

#include "platform/gl.h"

namespace pt
{
namespace gl
{

struct Vao
{
    Vao();

    GLuint id() const;

    Vao& bind();
    Vao& unbind();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace pt
