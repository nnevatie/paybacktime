#pragma once

#include <array>
#include <vector>
#include <iostream>

#include <glbinding/gl/types.h>
#include <glbinding/gl/enum.h>
using namespace gl;

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "rect.h"

namespace pt
{

struct VertexSpec
{
    // Component count, type, size in bytes
    using Attrib = std::tuple<int, GLenum, std::size_t>;

    std::size_t         size;
    std::vector<Attrib> attribs;
};

struct Vertex_P
{
    glm::vec3 p;

    static VertexSpec spec()
    {
        return {sizeof(Vertex_P),
               {std::make_tuple(3, GL_FLOAT, sizeof(p))}};
    }
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

struct Vertex_P_N_T_UV
{
    glm::vec3 p;
    glm::vec3 n;
    glm::vec3 t;
    glm::vec2 uv;

    static VertexSpec spec()
    {
        return {sizeof(Vertex_P_N_T_UV),
               {std::make_tuple(3, GL_FLOAT, sizeof(p)),
                std::make_tuple(3, GL_FLOAT, sizeof(n)),
                std::make_tuple(3, GL_FLOAT, sizeof(t)),
                std::make_tuple(2, GL_FLOAT, sizeof(uv))}};
    }
};

struct IndexSpec
{
    size_t size;
};

template <typename V, typename I>
struct Mesh
{
    using Vertex = V;
    using Index  = I;

    std::vector<V> vertices;
    std::vector<I> indices;

    Mesh() = default;

    Mesh(const std::vector<V>& vertices,
         const std::vector<I>& indices) :
         vertices(vertices), indices(indices) {}

    int triangleCount() const
    {
        return indices.size() / 3;
    }

    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
    {
        out << "Mesh[vertices: " << mesh.vertices.size()
            << ", triangles: " << mesh.triangleCount() << "]";

        return out;
    }
};

using Mesh_P        = Mesh<Vertex_P,        uint32_t>;
using Mesh_P_UV     = Mesh<Vertex_P_UV,     uint32_t>;
using Mesh_P_N_UV   = Mesh<Vertex_P_N_UV,   uint32_t>;
using Mesh_P_N_T_UV = Mesh<Vertex_P_N_T_UV, uint32_t>;

inline Mesh_P_UV rectMesh(float halfWidth = 1.f, float halfHeight = 1.f)
{
    return
    {
        {
            {glm::vec3(-halfWidth, -halfHeight, 0), glm::vec2(0, 0)},
            {glm::vec3( halfWidth, -halfHeight, 0), glm::vec2(1, 0)},
            {glm::vec3( halfWidth,  halfHeight, 0), glm::vec2(1, 1)},
            {glm::vec3(-halfWidth,  halfHeight, 0), glm::vec2(0, 1)},
        },
        {0, 1, 2, 2, 3, 0}
    };
}

inline Mesh_P triMesh(float halfWidth = 1.f)
{
    return
    {
        {
            {glm::vec3( halfWidth, 0,          0)},
            {glm::vec3( 0,         0, -halfWidth)},
            {glm::vec3(-halfWidth, 0,          0)},
        },
        {0, 1, 2}
    };
}

inline Mesh_P_UV squareMesh(float halfWidth = 1.f)
{
    return rectMesh(halfWidth, halfWidth);
}

inline Mesh_P gridMesh(float interval, float halfWidth, float halfHeight)
{
    const Size<int>   size(int(halfWidth * 2 / interval),
                           int(halfHeight * 2 / interval));
    const Size<float> step(halfWidth * 2 / size.w,
                           halfHeight * 2 / size.h);

    Mesh_P mesh;
    for (int y = 0; y <= size.h; ++y)
        for (int x = 0; x <= size.w; ++x)
        {
            if (x < size.w)
            {
                const auto index0 = Mesh_P::Index(mesh.vertices.size());
                mesh.indices.insert(mesh.indices.end(), {index0, index0 + 1});
                mesh.vertices.insert(mesh.vertices.end(),
                    {{{-halfWidth  + (x + 0) * step.w, 0,
                       -halfHeight + (y + 0) * step.h}},
                     {{-halfWidth  + (x + 1) * step.w, 0,
                       -halfHeight + (y + 0) * step.h}}});
            }
            if (y < size.h)
            {
                const auto index1 = Mesh_P::Index(mesh.vertices.size());
                mesh.indices.insert(mesh.indices.end(), {index1, index1 + 1});
                mesh.vertices.insert(mesh.vertices.end(),
                    {{{-halfWidth  + (x + 0) * step.w, 0,
                       -halfHeight + (y + 0) * step.h}},
                     {{-halfWidth  + (x + 0) * step.w, 0,
                       -halfHeight + (y + 1) * step.h}}});
            }
        }

    return mesh;
}

} // namespace
