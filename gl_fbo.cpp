#include <GL/glew.h>

#include "gl_fbo.h"

namespace hc
{
namespace gl
{
namespace
{
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

bool Fbo::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

} // namespace gl
} // namespace hc
