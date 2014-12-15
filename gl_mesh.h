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

    Buffer vertices,
           indices;
};

} // namespace gl
} // namespace hc
