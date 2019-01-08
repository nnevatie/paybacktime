#pragma once

#include <memory>

#include <glad/glad.h>

namespace pt
{
namespace gl
{

struct Buffer
{
    enum class Type
    {
        Vertex,
        Index,
        Texture
    };

    enum class Usage
    {
        StaticDraw,
        DynamicDraw,
        StreamDraw
    };

    Buffer();
    Buffer(Type type);

    operator bool() const;

    GLuint id() const;
    int size() const;

    Buffer& bind();
    Buffer& unbind();
    Buffer& alloc(const void* data, int size);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace pt
