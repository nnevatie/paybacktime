#pragma once

#include "mesh.h"
#include "gl_buffers.h"

namespace hc
{
namespace gl
{

struct Primitive
{
    enum class RenderType
    {
        Points,
        Lines,
        Triangles
    };

    explicit Primitive(const Mesh& mesh);

    void render(RenderType type = RenderType::Triangles) const;

    Buffer vertices, indices;
};

} // namespace gl
} // namespace hc
