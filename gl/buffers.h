#pragma once

#include <memory>

#include <glad/glad.h>

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

    int size() const;

    Buffer& bind();
    Buffer& unbind();
    Buffer& alloc(const void* data, int size);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace hc
