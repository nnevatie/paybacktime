#pragma once

#include <glbinding/gl/functions.h>

#include "common/common.h"
#include "geom/mesh.h"
#include "buffers.h"
#include "vao.h"

namespace pt
{
namespace gl
{

struct Primitive
{
    Primitive()
    {}

    template <typename V, typename I>
    explicit Primitive(const Mesh<V, I>& mesh) :
       vertexSpec {V::spec()}, indexSpec {sizeof(I)},
       vertices(Buffer::Type::Vertex),
       indices(Buffer::Type::Index)
    {
        Binder<gl::Vao> binder(vao);

        const std::size_t stride      = vertexSpec.size;
        const std::size_t attribCount = vertexSpec.attribs.size();

        vertices.alloc(mesh.vertices.data(),
                       int(sizeof(V) * mesh.vertices.size()));

        indices.alloc(mesh.indices.data(),
                      int(sizeof(I) * mesh.indices.size()));

        // Enable attrib arrays
        for (std::size_t i = 0; i < attribCount; ++i)
            glEnableVertexAttribArray(GLuint(i));

        // Set attrib pointers
        size_t offset = 0;
        for (std::size_t i = 0; i < attribCount; ++i)
        {
            const VertexSpec::Attrib& attrib = vertexSpec.attribs.at(i);
            const auto count = std::get<0>(attrib);
            const auto type  = std::get<1>(attrib);
            const auto size  = std::get<2>(attrib);

            if (type == GL_INT)
                glVertexAttribIPointer(GLuint(i), GLint(count), type,
                                       GLsizei(stride),
                                       reinterpret_cast<const void*>(offset));
            else
                glVertexAttribPointer(GLuint(i), GLint(count), type,
                                      GL_FALSE, GLsizei(stride),
                                      reinterpret_cast<const void*>(offset));
            offset += size;
        }
    }

    operator bool() const
    {
        return vertices;
    }

    void render(GLenum mode = GL_TRIANGLES, GLenum cull = GL_BACK) const
    {
        glEnable(GL_CULL_FACE);
        glCullFace(cull);

        // Draw elements with VAO
        Binder<gl::Vao> binder(vao);
        glDrawElements(mode,
                       indices.size() / int(indexSpec.size),
                       indexSpec.size == 4 ? GL_UNSIGNED_INT :
                       indexSpec.size == 2 ? GL_UNSIGNED_SHORT :
                                             GL_UNSIGNED_BYTE,
                       0);
    }

    VertexSpec  vertexSpec;
    IndexSpec   indexSpec;

    Buffer      vertices, indices;

    mutable
    Vao         vao;
};

} // namespace gl
} // namespace pt
