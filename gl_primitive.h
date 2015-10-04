#pragma once

#include "mesh.h"
#include "gl_buffers.h"

namespace hc
{
namespace gl
{

struct Primitive
{
    explicit Primitive(const Mesh& mesh);

    void render(GLenum mode = GL_TRIANGLES) const;

    Buffer vertices, indices;
};

} // namespace gl
} // namespace hc
