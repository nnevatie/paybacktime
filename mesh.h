#pragma once

#include <array>
#include <vector>
#include <iostream>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "geometry.h"

namespace hc
{

typedef std::array<glm::vec3, 8>        Box;
typedef std::pair<glm::vec3, glm::vec3> BoundingBox;

struct Vertex
{
    glm::vec3 p;
    glm::vec3 n;
    glm::vec2 uv;

    Vertex() {}
    Vertex(const glm::vec3& p) : p(p) {}
    Vertex(const glm::vec3& p, const glm::vec3& n) : p(p), n(n) {}
    Vertex(const glm::vec3& p, const glm::vec3& n, const glm::vec2& uv) :
        p(p), n(n), uv(uv) {}

    Vertex(float x,  float y,  float z) : p(x, y, z) {}
    Vertex(float x,  float y,  float z,
           float nx, float ny, float nz,
           float u,  float v) : p(x, y, z), n(nx, ny, nz), uv(u, v) {}
};

template <typename V = Vertex>
struct Mesh
{
    typedef uint32_t   Index;
    std::vector<Vertex> vertices;
    std::vector<Index>  indices;

    Mesh() {}

    Mesh(const std::vector<Vertex>& vertices,
         const std::vector<Index>& indices) :
         vertices(vertices), indices(indices) {}

    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
    {
        out << "Mesh[vertices: " << mesh.vertices.size()
            << ", triangles: " << mesh.indices.size() / 3 << "]";

        return out;
    }
};

inline Mesh<> rectMesh(float halfWidth = 1.f, float halfHeight = 1.f)
{
    return
    {
        {
            {-halfWidth, -halfHeight, 0, 0, 0, 1, 0, 0},
            { halfWidth, -halfHeight, 0, 0, 0, 1, 1, 0},
            { halfWidth,  halfHeight, 0, 0, 0, 1, 1, 1},
            {-halfWidth,  halfHeight, 0, 0, 0, 1, 0, 1},
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
