#pragma once

#include <memory>
#include <vector>

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
    Texture& unbind();

    Texture& alloc(const std::vector<int>& dim,
                   GLint internalFormat, GLenum format,
                   GLenum type = GL_UNSIGNED_BYTE, const GLvoid* data = 0);

    Texture& alloc(int level, const std::vector<int>& dim,
                   GLint internalFormat, GLenum format,
                   GLenum type = GL_UNSIGNED_BYTE, const GLvoid* data = 0);

    Texture& alloc(const Image& image);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace hc