#include <glad/glad.h>

#include "common/log.h"
#include "buffers.h"

namespace pt
{
namespace gl
{
namespace
{

GLenum bufferTarget(Buffer::Type type)
{
    return type == Buffer::Type::Vertex ? GL_ARRAY_BUFFER :
           type == Buffer::Type::Index  ? GL_ELEMENT_ARRAY_BUFFER :
                                          GL_TEXTURE_BUFFER;
}

GLenum bufferUsage(Buffer::Usage usage)
{
    switch (usage)
    {
        case Buffer::Usage::StaticDraw:
            return GL_STATIC_DRAW;
        case Buffer::Usage::DynamicDraw:
            return GL_DYNAMIC_DRAW;
        case Buffer::Usage::StreamDraw:
            return GL_STREAM_DRAW;
    }
    return 0;
}

} // namespace

struct Buffer::Data
{
    explicit Data(Buffer::Type type) :
        id(0), size(0), type(type), usage(Usage::StaticDraw)
    {
        glGenBuffers(1, &id);
    }

    ~Data()
    {
        glDeleteBuffers(1, &id);
    }

    GLuint id;
    int    size;
    Type   type;
    Usage  usage;
};

Buffer::Buffer(Buffer::Type type) :
    d(new Data(type))
{
}

GLuint Buffer::id() const
{
    return d->id;
}

int Buffer::size() const
{
    return d->size;
}

Buffer& Buffer::bind()
{
    glBindBuffer(bufferTarget(d->type), d->id);
    return *this;
}

Buffer& Buffer::unbind()
{
    glBindBuffer(bufferTarget(d->type), 0);
    return *this;
}

Buffer& Buffer::alloc(const void* data, int size)
{
    int allocated = 0;
    const GLenum target = bufferTarget(d->type);

    glBindBuffer(target, d->id);
    glBufferData(target, size, data, bufferUsage(d->usage));
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &allocated);

    if(allocated == size)
        d->size = size;
    else
        HCLOG(Warn) << "Could not alloc " << size << " bytes.";

    return *this;
}

} // namespace gl
} // namespace pt
