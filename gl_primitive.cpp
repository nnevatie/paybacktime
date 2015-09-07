#include "gl_primitive.h"

namespace hc
{
namespace gl
{

Primitive::Primitive(const Mesh& mesh) :
    vertices(Buffer::Type::Vertex),
    indices(Buffer::Type::Index)
{
    vertices.alloc(mesh.vertices.data(),
                   int(sizeof(Mesh::Vertex) * mesh.vertices.size()));

    indices.alloc(mesh.indices.data(),
                  int(sizeof(Mesh::Index) * mesh.indices.size()));
}

void Primitive::render(RenderType type) const
{
    const GLenum mode = type == RenderType::Points ? GL_POINT :
                        type == RenderType::Lines  ? GL_LINE : GL_FILL;

    glPolygonMode(GL_FRONT_AND_BACK, mode);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    vertices.bind();

    for (int i = 0; i < 3; ++i)
        glEnableVertexAttribArray(i);

    const int stride = sizeof(Mesh::Vertex);
    auto offset = [](size_t size) {return (const void*) size;};

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          stride, offset(sizeof(float) * 0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          stride, offset(sizeof(float) * 3));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          stride, offset(sizeof(float) * 6));

    indices.bind();
    glDrawElements(GL_TRIANGLES,
                   indices.size / int(sizeof(Mesh::Index)),
                   GL_UNSIGNED_INT,
                   0);

    for (int i = 0; i < 3; ++i)
        glDisableVertexAttribArray(i);

    indices.unbind();
    vertices.unbind();
}

} // namespace gl
} // namespace hc
