#include "gl_mesh.h"

#include "log.h"

namespace hc
{
namespace gl
{

Mesh::Mesh(const Geometry& geometry) :
    vertices(Buffer::Type::Vertex),
    indices(Buffer::Type::Index)
{
    HCLOG(Info) << geometry.vertices.size()
                << " "
                << sizeof(Geometry::Vertex);

    vertices.alloc(geometry.vertices.data(),
                   int(sizeof(Geometry::Vertex) * geometry.vertices.size()));

    indices.alloc(geometry.indices.data(),
                  int(sizeof(Geometry::Index) * geometry.indices.size()));
}

} // namespace gl
} // namespace hc
