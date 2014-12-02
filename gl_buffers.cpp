#include <GL/glew.h>

#include "gl_buffers.h"
#include "log.h"

namespace hc
{
namespace gl
{
namespace
{

GLenum bufferTarget(Buffer::Type type)
{
    return type == Buffer::Type::Vertex ? GL_ARRAY_BUFFER_ARB :
                                          GL_ELEMENT_ARRAY_BUFFER_ARB;
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
    id(0)
{
}

Buffer::~Buffer()
{
}

bool Buffer::alloc(const void* data, int size)
{
    int allocated = 0;
    const GLenum target = bufferTarget(type);

    glGenBuffers(1, &id);
    glBindBuffer(target, id);
    glBufferData(target, size, data, bufferUsage(usage));
    glGetBufferParameterivARB(target, GL_BUFFER_SIZE_ARB, &allocated);

    if(allocated != size)
    {
        HCLOG(Warn) << "Could not alloc " << size << " bytes.";
        glDeleteBuffersARB(1, &id);
        id = 0;
        return false;
    }
    return true;
}

} // namespace gl
} // namespace hc
