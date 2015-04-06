#pragma once

#include "geometry.h"
#include "gl_buffers.h"

namespace hc
{
namespace gl
{

struct Mesh
{
    enum class RenderType
    {
        Points,
        Lines,
        Triangles
    };

    Mesh(const Geometry& geometry);

    void render(RenderType type = RenderType::Triangles) const;

    Buffer vertices,
           indices;
};

} // namespace gl
} // namespace hc
