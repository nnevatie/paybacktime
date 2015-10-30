#pragma once

#include "mesh.h"
#include "gl_buffers.h"

namespace hc
{
namespace gl
{

template <typename V = Vertex>
struct Primitive
{
    explicit Primitive(const Mesh<V>& mesh) :
       vertices(Buffer::Type::Vertex),
       indices(Buffer::Type::Index)
    {
        vertices.alloc(mesh.vertices.data(),
                       int(sizeof(V) * mesh.vertices.size()));

        indices.alloc(mesh.indices.data(),
                      int(sizeof(V) * mesh.indices.size()));
    }

    void render(GLenum mode = GL_TRIANGLES) const
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        vertices.bind();

        for (int i = 0; i < 3; ++i)
            glEnableVertexAttribArray(i);

        const int stride = sizeof(V);
        auto offset = [](size_t size) {return (const void*) size;};

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              stride, offset(sizeof(float) * 0));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              stride, offset(sizeof(float) * 3));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                              stride, offset(sizeof(float) * 6));

        indices.bind();

        glDrawElements(mode,
                       indices.size / int(sizeof(Mesh<>::Index)),
                       GL_UNSIGNED_INT,
                       0);

        for (int i = 0; i < 3; ++i)
            glDisableVertexAttribArray(i);

        indices.unbind();
        vertices.unbind();
    }

    Buffer vertices, indices;
};

} // namespace gl
} // namespace hc
