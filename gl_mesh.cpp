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
    glEnableClientState(GL_VERTEX_ARRAY);

    const GLenum mode = type == RenderType::Points ? GL_POINT :
                        type == RenderType::Lines  ? GL_LINE : GL_FILL;

    glPolygonMode(GL_FRONT_AND_BACK, mode);

    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    vertices.bind();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    const int stride  = sizeof(GLfloat) * 8;
    glVertexPointer(3,   GL_FLOAT, stride, 0);
    glNormalPointer(     GL_FLOAT, stride, (float*) (sizeof(float) * 3));
    glTexCoordPointer(2, GL_FLOAT, stride, (float*) (sizeof(float) * 6));

    indices.bind();
    glDrawElements(GL_TRIANGLES,
                   indices.size / int(sizeof(Geometry::Index)),
                   GL_UNSIGNED_INT,
                   0);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    vertices.unbind();
    indices.unbind();

    glDisableClientState(GL_VERTEX_ARRAY);
}

} // namespace gl
} // namespace hc
