#include "gl_mesh.h"

namespace hc
{
namespace gl
{

Mesh::Mesh(const Geometry& geometry) :
    vertices(Buffer::Type::Vertex),
    indices(Buffer::Type::Index)
{
    vertices.alloc(geometry.vertices.data(),
                   int(sizeof(Geometry::Vertex) * geometry.vertices.size()));

    indices.alloc(geometry.indices.data(),
                  int(sizeof(Geometry::Index) * geometry.indices.size()));
}

void Mesh::render() const
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnableClientState(GL_VERTEX_ARRAY);

    vertices.bind();
    glVertexPointer(3, GL_FLOAT, 0, 0);

    indices.bind();
    glDrawElements(GL_TRIANGLES,
                   indices.size / int(sizeof(GLushort)),
                   GL_UNSIGNED_SHORT,
                   0);

    vertices.unbind();
    indices.unbind();

    glDisableClientState(GL_VERTEX_ARRAY);
}

} // namespace gl
} // namespace hc
