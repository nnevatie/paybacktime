#pragma once

#include <GL/glew.h>

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
        StaticDraw,
        DynamicDraw,
        StreamDraw
    };

    Buffer(Type type);
    ~Buffer();

    bool bind() const;
    bool unbind() const;

    bool alloc(const void* data, int size);
    bool dealloc();

    // TODO: Make shared private
    Type   type;
    Usage  usage;
    GLuint id;
    int    size;
};

} // namespace gl
} // namespace hc
