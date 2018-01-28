#include "texture.h"

#include <glbinding/gl/functions.h>

#include <glm/gtc/constants.hpp>

#include "common/log.h"

namespace pt
{
namespace gl
{
namespace
{

GLenum textureTarget(Texture::Type type)
{
    const std::array<GLenum, 7> types =
    {
        GL_TEXTURE_1D,
        GL_TEXTURE_2D,
        GL_TEXTURE_2D_MULTISAMPLE,
        GL_TEXTURE_3D,
        GL_TEXTURE_1D_ARRAY,
        GL_TEXTURE_2D_ARRAY,
        GL_TEXTURE_BUFFER
    };
    const auto index = std::size_t(type);

    if (index >= int(types.size()))
        throw std::runtime_error("Invalid texture type " + std::to_string(index));

    return types.at(index);
}

} // namespace

struct Texture::Data
{
    explicit Data(Texture::Type type, int sampleCount) :
        id(0), target(textureTarget(type)),
        type(type), sampleCount(sampleCount)
    {
        glGenTextures(1, &id);
    }

    ~Data()
    {
        glDeleteTextures(1, &id);
    }

    GLuint        id;
    GLenum        target;
    Texture::Type type;
    int           sampleCount;
};

Texture::Texture(Type type, int sampleCount) :
    d(std::make_shared<Data>(type, sampleCount))
{
}

pt::gl::Texture::operator bool()
{
    return size() != glm::zero<glm::ivec3>();
}

GLuint Texture::id() const
{
    return d->id;
}

Texture::Type Texture::type() const
{
    return d->type;
}

glm::ivec3 Texture::size()
{
    Binder<gl::Texture> binder(this);
    glm::ivec3 size;
    glGetTexLevelParameteriv(d->target, 0, GL_TEXTURE_WIDTH,  &size.x);
    glGetTexLevelParameteriv(d->target, 0, GL_TEXTURE_HEIGHT, &size.y);
    glGetTexLevelParameteriv(d->target, 0, GL_TEXTURE_DEPTH,  &size.z);
    return size;
}

int Texture::dimensions() const
{
    switch (d->target)
    {
        case GL_TEXTURE_1D:
            return 1;
        case GL_TEXTURE_2D:
        case GL_TEXTURE_2D_MULTISAMPLE:
            return 2;
        case GL_TEXTURE_3D:
            return 3;
        default:
            return 2;
    }
}

int Texture::sampleCount() const
{
    return d->sampleCount;
}

Image Texture::image()
{
    const auto s = size();
    Image image(Size<int>(s.x, s.y), 4);
    Binder<gl::Texture> binder(this);
    glGetTexImage(d->target, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    return image;
}

Texture& Texture::bind()
{
    glBindTexture(d->target, d->id);
    return *this;
}

Texture& Texture::bindAs(GLenum unit)
{
    glActiveTexture(unit);
    return bind();
}

Texture& Texture::unbind()
{
    glBindTexture(d->target, 0);
    return *this;
}

void Texture::unbind(GLenum target, GLenum unit)
{
    glActiveTexture(unit);
    glBindTexture(target, 0);
}

Texture& Texture::set(GLenum name, GLenum param)
{
    glTexParameteri(d->target, name, param);
    return *this;
}

Texture& Texture::set(GLenum name, GLint param)
{
    glTexParameteri(d->target, name, param);
    return *this;
}

Texture& Texture::set(GLenum name, GLfloat param)
{
    glTexParameterf(d->target, name, param);
    return *this;
}

Texture& Texture::alloc(GLenum internalFormat, const Buffer& buffer)
{
    glTexBuffer(GL_TEXTURE_BUFFER, internalFormat, buffer.id());
    return *this;
}

Texture& Texture::alloc(const std::vector<int>& dim,
                        GLenum internalFormat, GLenum format,
                        GLenum type, const GLvoid* data)
{
    return alloc(0, dim, internalFormat, format, type, data);
}

Texture& Texture::alloc(int level, const std::vector<int>& dim,
                        GLenum internalFormat, GLenum format,
                        GLenum type, const GLvoid* data)
{
    if (d->target == GL_TEXTURE_1D && dim.size() > 0)
        glTexImage1D(d->target,
                     level, internalFormat,
                     dim[0], 0, format, type, data);
    else
    if (d->target == GL_TEXTURE_2D && dim.size() > 1)
        glTexImage2D(d->target,
                     level, internalFormat,
                     dim[0], dim[1], 0, format, type, data);
    else
    if (d->target == GL_TEXTURE_2D_MULTISAMPLE && dim.size() > 1)
        glTexImage2DMultisample(d->target, d->sampleCount,
                                internalFormat,
                                dim[0], dim[1], GL_TRUE);
    else
    if (d->target == GL_TEXTURE_3D && dim.size() > 2)
        glTexImage3D(d->target,
                     level, internalFormat,
                     dim[0], dim[1], dim[2], 0, format, type, data);

    // Set default params
    if (d->target != GL_TEXTURE_2D_MULTISAMPLE)
    {
        set(GL_TEXTURE_MIN_FILTER, GLint(GL_NEAREST));
        set(GL_TEXTURE_MAG_FILTER, GLint(GL_NEAREST));
        set(GL_TEXTURE_WRAP_S,     GLint(GL_CLAMP_TO_EDGE));
        set(GL_TEXTURE_WRAP_T,     GLint(GL_CLAMP_TO_EDGE));
        set(GL_TEXTURE_WRAP_R,     GLint(GL_CLAMP_TO_EDGE));
    }
    return *this;
}

Texture& Texture::alloc(const Image& image, bool srgb)
{
    struct Format
    {
        GLenum internalFormat;
        GLenum format;
    }
    const formats[] =
    {
        {GL_R8,                             GL_RED},
        {GL_RG8,                            GL_RG},
        {srgb ? GL_SRGB8        : GL_RGB8,  GL_RGB},
        {srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8, GL_RGBA}
    };
    if (image.depth() > 0 && image.depth() <= 4)
    {
        const Format f = formats[image.depth() - 1];
        return alloc({image.size().w, image.size().h},
                      f.internalFormat, f.format, GL_UNSIGNED_BYTE, image.bits());
    }
    return *this;
}

Texture& Texture::alloc(const Grid<float>& grid)
{
    return alloc(grid.dims(), GL_R32F, GL_RED, GL_FLOAT, grid.ptr());
}

Texture& Texture::alloc(const Grid<glm::vec3>& grid)
{
    return alloc(grid.dims(), GL_RGB32F, GL_RGB, GL_FLOAT, grid.ptr());
}

Texture& Texture::alloc(const Grid<glm::vec4>& grid)
{
    return alloc(grid.dims(), GL_RGBA32F, GL_RGBA, GL_FLOAT, grid.ptr());
}

float Texture::anisotropyMax()
{
    GLfloat f = 0.f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &f);
    return f;
}

} // namespace gl
} // namespace pt
