#include <GL/glew.h>

#include "gl_texture.h"

namespace hc
{
namespace gl
{
namespace
{

GLenum textureTarget(Texture::Type type)
{
    const std::array<GLenum, 5> types =
    {
        GL_TEXTURE_1D,
        GL_TEXTURE_2D,
        GL_TEXTURE_3D,
        GL_TEXTURE_1D_ARRAY,
        GL_TEXTURE_2D_ARRAY
    };
    const int index = int(type);

    if (index < 0 || index >= int(types.size()))
        throw std::runtime_error("Invalid texture type " + std::to_string(index));

    return types.at(index);
}

} // namespace

struct Texture::Data
{
    Data(Texture::Type type) : id(0), target(textureTarget(type)), type(type)
    {
        glGenTextures(1, &id);
    }

    ~Data()
    {
        glDeleteTextures(1, &id);
    }

    GLuint id;
    GLenum target;
    Texture::Type type;
};

Texture::Texture(Type type) :
    d(new Data(type))
{
}

GLuint Texture::id() const
{
    return d->id;
}

Texture::Type Texture::type() const
{
    return d->type;
}

Texture& Texture::bind()
{
    glBindTexture(d->target, d->id);
    return *this;
}

bool Texture::unbind()
{
    glBindTexture(d->target, 0);
    return true;
}

Texture& Texture::alloc(int width, int height, const GLvoid* data)
{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    return *this;
}

Texture &Texture::alloc(const Image& image)
{
    return *this;
}

} // namespace gl
} // namespace hc
