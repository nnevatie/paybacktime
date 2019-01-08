#pragma once

#include <memory>

#include <glad/glad.h>

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
