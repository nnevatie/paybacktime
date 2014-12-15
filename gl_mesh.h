#pragma once

#include "geometry.h"
#include "gl_buffers.h"

namespace hc
{
namespace gl
{

struct Mesh
{
    Mesh(const Geometry& geometry);

    void render() const;

    Buffer vertices,
           indices;
};

} // namespace gl
} // namespace hc
