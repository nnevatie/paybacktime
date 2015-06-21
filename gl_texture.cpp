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

Texture& Texture::alloc(const std::vector<int> &dim,
                        GLint internalFormat, GLenum format, const GLvoid* data)
{
    return alloc(0, dim, internalFormat, format, data);
}

Texture& Texture::alloc(int level, const std::vector<int>& dim,
                        GLint internalFormat, GLenum format, const GLvoid* data)
{
    const GLenum type = GL_UNSIGNED_BYTE;

    if (d->target == GL_TEXTURE_1D && dim.size() > 0)
        glTexImage1D(GL_TEXTURE_1D,
                     level, internalFormat,
                     dim[0], 0, format, type , data);
    else
    if (d->target == GL_TEXTURE_2D && dim.size() > 1)
        glTexImage2D(GL_TEXTURE_2D,
                     level, internalFormat,
                     dim[0], dim[1], 0, format, type , data);
    else
    if (d->target == GL_TEXTURE_3D && dim.size() > 2)
        glTexImage3D(GL_TEXTURE_3D,
                     level, internalFormat,
                     dim[0], dim[1], dim[2], 0, format, type , data);
    return *this;
}

Texture &Texture::alloc(const Image& image)
{
    return *this;
}

} // namespace gl
} // namespace hc
