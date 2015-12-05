#include <glad/glad.h>

#include "buffers.h"
#include "log.h"

namespace hc
{
namespace gl
{
namespace
{

GLenum bufferTarget(Buffer::Type type)
{
    return type == Buffer::Type::Vertex ? GL_ARRAY_BUFFER :
                                          GL_ELEMENT_ARRAY_BUFFER;
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

}

Buffer::Buffer(Buffer::Type type) :
    type(type),
    usage(Usage::StaticDraw),
    id(0),
    size(0)
{
}

Buffer::~Buffer()
{
    dealloc();
}

bool Buffer::bind() const
{
    if (id)
    {
        glBindBuffer(bufferTarget(type), id);
        return true;
    }
    return false;
}

bool Buffer::unbind() const
{
    if (id)
    {
        glBindBuffer(bufferTarget(type), 0);
        return true;
    }
    return false;
}

bool Buffer::alloc(const void* data, int size)
{
    int allocated = 0;
    const GLenum target = bufferTarget(type);

    glGenBuffers(1, &id);
    glBindBuffer(target, id);
    glBufferData(target, size, data, bufferUsage(usage));
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &allocated);

    if(allocated != size)
    {
        HCLOG(Warn) << "Could not alloc " << size << " bytes.";
        dealloc();
        return false;
    }

    this->size = size;
    return true;
}

bool Buffer::dealloc()
{
    if (id)
    {
        glDeleteBuffers(1, &id);
        id = 0;
        return true;
    }
    return false;
}

} // namespace gl
} // namespace hc
