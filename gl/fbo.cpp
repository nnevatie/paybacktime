#include <glad/glad.h>

#include "fbo.h"

namespace pt
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
    const std::size_t index = std::size_t(attachment);

    if (index >= types.size())
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
    d(std::make_shared<Data>())
{
}

Fbo& Fbo::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, d->id);
    return *this;
}

Fbo& Fbo::attach(const Rbo& rbo, Fbo::Attachment attachment, int index)
{
    const GLenum type = GLenum(attachmentType(attachment) + uint32_t(index));
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, type, GL_RENDERBUFFER, rbo.id());
    return *this;
}

Fbo& Fbo::attach(const Texture& texture, Attachment attachment,
                 int index, int level, int layer)
{
    const GLenum target = GL_FRAMEBUFFER;
    const GLenum type   = GLenum(attachmentType(attachment) + uint32_t(index));
    const GLuint tid    = texture.id();

    if (texture.type() == Texture::Type::Texture1d)
        glFramebufferTexture1D(target, type, GL_TEXTURE_1D, tid, level);
    else
    if (texture.type() == Texture::Type::Texture2d)
        glFramebufferTexture2D(target, type, GL_TEXTURE_2D, tid, level);
    else
    if (texture.type() == Texture::Type::Texture3d)
        glFramebufferTexture3D(target, type, GL_TEXTURE_3D, tid, level, layer);

    return *this;
}

bool Fbo::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

} // namespace gl
} // namespace pt
