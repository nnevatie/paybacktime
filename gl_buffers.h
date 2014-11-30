#pragma once

namespace hc
{
namespace gl
{

struct Buffer
{
    enum class Type
    {
        TypeVertex,
        TypeIndex
    };

    Buffer(Type type);
    ~Buffer();

    Type type;
};

} // namespace gl
} // namespace hc
