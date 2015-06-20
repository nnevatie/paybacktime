#include <GL/glew.h>

#include "gl_fbo.h"

namespace hc
{
namespace gl
{
namespace
{

GLenum attachmentType(Fbo::Attachment attachment)
{
    const std::array<GLenum, 4> types =
    {
        GL_COLOR_ATTACHMENT0,
        GL_DEPTH_ATTACHMENT,
        GL_STENCIL_ATTACHMENT,
        GL_DEPTH_STENCIL_ATTACHMENT
    };
    const int index = int(attachment);

    if (index < 0 || index >= int(types.size()))
        throw std::runtime_error("Invalid attachment type " +
                                 std::to_string(index));

    return types.at(index);
}

} // namespace

struct Fbo::Data
{
    Data() : id(0)
    {
        glGenFramebuffers(1, &id);
    }

    ~Data()
    {
        glDeleteFramebuffers(1, &id);
    }

    GLuint id;
};

Fbo::Fbo() :
    d(new Data())
{
}

Fbo& Fbo::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, d->id);
    return *this;
}

Fbo& Fbo::attach(const Texture& texture, Attachment attachment, int index)
{
    const GLenum target = GL_FRAMEBUFFER;
    const GLenum type   = GLenum(attachmentType(attachment) + index);
    const GLuint tid    = texture.id();

    if (texture.type() == Texture::Type::Texture1d)
        glFramebufferTexture1D(target, type, GL_TEXTURE_1D, tid, 0);
    else
    if (texture.type() == Texture::Type::Texture2d)
        glFramebufferTexture2D(target, type, GL_TEXTURE_2D, tid, 0);
    else
    if (texture.type() == Texture::Type::Texture3d)
        glFramebufferTexture3D(target, type, GL_TEXTURE_3D, tid, 0, 0); // TODO

    return *this;
}

bool Fbo::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

} // namespace gl
} // namespace hc
