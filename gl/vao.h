#pragma once

#include <memory>

#include <glbinding/gl/types.h>
using namespace gl;

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
