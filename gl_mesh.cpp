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

void Mesh::render(RenderType type) const
{
    const GLenum mode = type == RenderType::Points ? GL_POINT :
                        type == RenderType::Lines  ? GL_LINE : GL_FILL;

    glPolygonMode(GL_FRONT_AND_BACK, mode);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    vertices.bind();

    for (int i = 0; i < 3; ++i)
        glEnableVertexAttribArray(i);

    const int stride = sizeof(Geometry::Vertex);
    auto offset = [](size_t size) {return (const void*) size;};

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          stride, offset(sizeof(float) * 0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          stride, offset(sizeof(float) * 3));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          stride, offset(sizeof(float) * 6));

    indices.bind();
    glDrawElements(GL_TRIANGLES,
                   indices.size / int(sizeof(Geometry::Index)),
                   GL_UNSIGNED_INT,
                   0);

    for (int i = 0; i < 3; ++i)
        glDisableVertexAttribArray(i);

    indices.unbind();
    vertices.unbind();

    glDisableClientState(GL_VERTEX_ARRAY);
}

} // namespace gl
} // namespace hc
