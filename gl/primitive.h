#pragma once

#include "geom/mesh.h"
#include "buffers.h"

namespace hc
{
namespace gl
{

struct Primitive
{
    template <typename V, typename I>
    explicit Primitive(const Mesh<V, I>& mesh) :
       vertexSpec {V::spec()}, indexSpec {sizeof(I)},
       vertices(Buffer::Type::Vertex),
       indices(Buffer::Type::Index),
       vao(0)
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        const std::size_t stride      = vertexSpec.size;
        const std::size_t attribCount = vertexSpec.attribs.size();

        vertices.alloc(mesh.vertices.data(),
                       int(sizeof(V) * mesh.vertices.size()));

        indices.alloc(mesh.indices.data(),
                      int(sizeof(I) * mesh.indices.size()));

        // Enable attrib arrays
        for (std::size_t i = 0; i < attribCount; ++i)
            glEnableVertexAttribArray(i);

        // Set attrib pointers
        size_t offset = 0;
        for (std::size_t i = 0; i < attribCount; ++i)
        {
            const VertexSpec::Attrib& attrib = vertexSpec.attribs.at(i);
            glVertexAttribPointer(i, std::get<0>(attrib), std::get<1>(attrib),
                                  GL_FALSE, stride,
                                  reinterpret_cast<const void*>(offset));
            offset += std::get<2>(attrib);
        }

        glBindVertexArray(0);
    }

    void render(GLenum mode = GL_TRIANGLES) const
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Draw elements with VAO
        glBindVertexArray(vao);
        glDrawElements(mode,
                       indices.size / int(indexSpec.size),
                       indexSpec.size == 4 ? GL_UNSIGNED_INT :
                       indexSpec.size == 2 ? GL_UNSIGNED_SHORT :
                                             GL_UNSIGNED_BYTE,
                       0);
        glBindVertexArray(0);
    }

    VertexSpec vertexSpec;
    IndexSpec  indexSpec;

    Buffer     vertices, indices;
    GLuint     vao;
};

} // namespace gl
} // namespace hc
