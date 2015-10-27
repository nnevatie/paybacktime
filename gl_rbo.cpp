#include <GL/glew.h>

#include "gl_rbo.h"

namespace hc
{
namespace gl
{

struct Rbo::Data
{
    Data() : id(0)
    {
        glGenRenderbuffers(1, &id);
    }

    ~Data()
    {
        glDeleteRenderbuffers(1, &id);
    }

    GLuint id;
};

Rbo::Rbo() :
    d(new Data())
{
}

GLuint Rbo::id() const
{
    return d->id;
}

Rbo& Rbo::bind()
{
    glBindRenderbuffer(GL_RENDERBUFFER, d->id);
    return *this;
}

Rbo& Rbo::unbind()
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return *this;
}

Rbo& Rbo::alloc(const Size<int>& size, GLint internalFormat)
{
    glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, size.w, size.h);
    return *this;
}

} // namespace gl
} // namespace hc
