#pragma once

#include <GL/gl.h>

namespace hc
{
namespace gl
{

struct Buffer
{
    enum class Type
    {
        Vertex,
        Index
    };

    enum class Usage
    {
        StaticDraw
        DynamicDraw,
        StreamDraw
    };,

    Buffer(Type type);
    ~Buffer();

    bool alloc(const void* data, int size);
    bool dealloc();

    Type   type;
    Usage  usage;
    GLuint id;
};

} // namespace gl
} // namespace hc
