#include "vao.h"

#include <glbinding/gl/functions.h>

namespace pt
{
namespace gl
{

struct Vao::Data
{
    Data() : id(0)
    {
        glGenVertexArrays(1, &id);
    }

    ~Data()
    {
        glDeleteVertexArrays(1, &id);
    }

    GLuint id;
};

Vao::Vao() :
    d(std::make_shared<Data>())
{
}

GLuint Vao::id() const
{
    return d->id;
}

Vao& Vao::bind()
{
    glBindVertexArray(d->id);
    return *this;
}

Vao& Vao::unbind()
{
    glBindVertexArray(0);
    return *this;
}

} // namespace gl
} // namespace pt
