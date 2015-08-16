#include <GL/glew.h>

#include "gl_texture.h"

#include "log.h"

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

Texture& Texture::unbind()
{
    glBindTexture(d->target, 0);
    return *this;
}

Texture& Texture::alloc(const std::vector<int> &dim,
                        GLint internalFormat, GLenum format,
                        GLenum type, const GLvoid* data)
{
    return alloc(0, dim, internalFormat, format, type, data);
}

Texture& Texture::alloc(int level, const std::vector<int>& dim,
                        GLint internalFormat, GLenum format,
                        GLenum type, const GLvoid* data)
{
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

    // TODO: Move to a method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return *this;
}

Texture& Texture::alloc(const Image& image)
{
    struct Format
    {
        GLint internalFormat;
        GLenum format;
    }
    formats[] =
    {
        {GL_R,    GL_RED},
        {GL_RG,   GL_RG},
        {GL_RGB,  GL_RGB},
        {GL_RGBA, GL_RGBA}
    };
    if (image.depth() > 0 && image.depth() <= 4)
    {
        const Format f = formats[image.depth() - 1];
        return alloc({image.rect().w, image.rect().h},
                      f.internalFormat, f.format, GL_UNSIGNED_BYTE, image.bits());
    }
    return *this;
}

} // namespace gl
} // namespace hc