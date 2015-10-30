#pragma once

#include <array>
#include <vector>
#include <iostream>

#include <gl/glew.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "geometry.h"

namespace hc
{

typedef std::array<glm::vec3, 8>        Box;
typedef std::pair<glm::vec3, glm::vec3> BoundingBox;

struct VertexSpec
{
    // Component count, type, size in bytes
    typedef std::tuple<int, GLenum, std::size_t> Attrib;

    std::size_t         size;
    std::vector<Attrib> attribs;
};

struct Vertex_P_UV
{
    glm::vec3 p;
    glm::vec2 uv;

    static VertexSpec spec()
    {
        return {sizeof(Vertex_P_UV),
               {std::make_tuple(3, GL_FLOAT, sizeof(p)),
                std::make_tuple(2, GL_FLOAT, sizeof(uv))}};
    }
};

struct Vertex_P_N_UV
{
    glm::vec3 p;
    glm::vec3 n;
    glm::vec2 uv;

    static VertexSpec spec()
    {
        return {sizeof(Vertex_P_N_UV),
               {std::make_tuple(3, GL_FLOAT, sizeof(p)),
                std::make_tuple(3, GL_FLOAT, sizeof(n)),
                std::make_tuple(2, GL_FLOAT, sizeof(uv))}};
    }
};

struct IndexSpec
{
    size_t size;
};

template <typename V = Vertex_P_N_UV, typename I = uint32_t>
struct Mesh
{
    typedef V Vertex;
    typedef I Index;

    std::vector<V> vertices;
    std::vector<I> indices;

    Mesh() {}

    Mesh(const std::vector<V>& vertices,
         const std::vector<I>& indices) :
         vertices(vertices), indices(indices) {}

    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
    {
        out << "Mesh[vertices: " << mesh.vertices.size()
            << ", triangles: " << mesh.indices.size() / 3 << "]";

        return out;
    }
};

typedef Mesh<Vertex_P_UV,   uint32_t> Mesh_P_UV;
typedef Mesh<Vertex_P_N_UV, uint32_t> Mesh_P_N_UV;

inline Mesh<> rectMesh(float halfWidth = 1.f, float halfHeight = 1.f)
{
    const glm::vec3 n(0, 0, 1);
    return
    {
        {
            {glm::vec3(-halfWidth, -halfHeight, 0), n, glm::vec2(0, 0)},
            {glm::vec3( halfWidth, -halfHeight, 0), n, glm::vec2(1, 0)},
            {glm::vec3( halfWidth,  halfHeight, 0), n, glm::vec2(1, 1)},
            {glm::vec3(-halfWidth,  halfHeight, 0), n, glm::vec2(0, 1)},
        },
        {0, 1, 2, 2, 3, 0}
    };
}

inline Mesh<> squareMesh(float halfWidth = 1.f)
{
    return rectMesh(halfWidth, halfWidth);
}

inline Mesh<> gridMesh(float interval, float halfWidth, float halfHeight)
{
    const Size<int>   size(halfWidth * 2 / interval, halfHeight * 2 / interval);
    const Size<float> step(halfWidth * 2 / size.w,   halfHeight * 2 / size.h);

    Mesh<> mesh;
    for (int y = 0; y <= size.h; ++y)
        for (int x = 0; x <= size.w; ++x)
        {
            if (x < size.w)
            {
                const Mesh<>::Index index0 = mesh.vertices.size();
                mesh.indices.insert(mesh.indices.end(), {index0, index0 + 1});
                mesh.vertices.insert(mesh.vertices.end(),
                    {{-halfWidth  + (x + 0) * step.w, 0,
                      -halfHeight + (y + 0) * step.h},
                     {-halfWidth  + (x + 1) * step.w, 0,
                      -halfHeight + (y + 0) * step.h}});
            }
            if (y < size.h)
            {
                const Mesh<>::Index index1 = mesh.vertices.size();
                mesh.indices.insert(mesh.indices.end(), {index1, index1 + 1});
                mesh.vertices.insert(mesh.vertices.end(),
                    {{-halfWidth  + (x + 0) * step.w, 0,
                      -halfHeight + (y + 0) * step.h},
                     {-halfWidth  + (x + 0) * step.w, 0,
                      -halfHeight + (y + 1) * step.h}});
            }
        }

    return mesh;
}

} // namespace
