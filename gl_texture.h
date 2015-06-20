#pragma once

#include <memory>

#include "image.h"

namespace hc
{
namespace gl
{

struct Texture
{
    enum class Type
    {
        Texture1d,
        Texture2d,
        Texture3d,
        Array1d,
        Array2d
    };

    Texture(Type type = Type::Texture2d);

    GLuint id() const;
    Type type() const;

    Texture& bind();

    bool unbind();

    Texture& alloc(int width, int height, const GLvoid* data = 0);
    Texture& alloc(const Image& image);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace hc
