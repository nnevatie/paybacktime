#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

#include "platform/clock.h"
#include "common/log.h"

#include "mesh_common.h"
#include "mesh.h"

namespace pt
{
namespace MeshDeformer
{

// Common types
using Indices       = std::vector<int>;
using PosNormal     = std::pair<glm::vec3, glm::vec3>;
using Locations     = std::unordered_map<glm::vec3, Indices>;
using Locations2    = std::unordered_map<PosNormal, Indices>;
using Neighbors     = std::vector<Indices>;
using Displacements = std::vector<glm::vec3>;

struct Triangle;
struct Vertex;

struct Triangle
{
    Vertex*   vertices[3];
    glm::vec3 normal;

    Triangle(Vertex* v0, Vertex* v1, Vertex* v2);
    ~Triangle();

    bool      contains(const Vertex* v) const;
    bool      updateNormal();
    Triangle& replace(Vertex* v0, Vertex* v1);
};
using Triangles = std::vector<Triangle*>;

struct Vertex
{
    using Neighbors = std::vector<Vertex*>;
    using Triangles = std::vector<Triangle*>;

    Vertex(const glm::vec3& pos, int id);
    ~Vertex();

    bool removeNonNeighbor(Vertex* v);

    glm::vec3 pos;
    int       id;
    Neighbors neighbors;
    Triangles triangles;
    float     cost;
    Vertex*   collapse;
};
using Vertices = std::vector<Vertex*>;

template <typename M>
M connect(const M& mesh0)
{
    PTTIMEU("connect", boost::milli);

    const auto vc0 = int(mesh0.vertices.size());

    Locations2 locations(vc0 / 8);
    for (int i = 0; i < vc0; ++i)
        locations[{mesh0.vertices[i].p, mesh0.vertices[i].n}].emplace_back(i);

    std::vector<int> indexMap(vc0);
    std::vector<typename M::Vertex> vertexMap(locations.size());

    auto vertex = [&](const std::vector<int>& indices)
    {
        glm::vec3 p(glm::zero<glm::vec3>()),
                  n(glm::zero<glm::vec3>()),
                  t(glm::zero<glm::vec3>());
        glm::vec2 uv(glm::zero<glm::vec2>());

        for (const auto i : indices)
        {
            p  += mesh0.vertices[i].p;
            n  += mesh0.vertices[i].n;
            t  += mesh0.vertices[i].t;
            uv += mesh0.vertices[i].uv;
        }
        const float c = float(indices.size());
        return typename M::Vertex {p / c, n / c, t / c, uv / c};
    };

    int index = 0;
    for (const auto& loc : locations)
    {
        for (const auto i : loc.second)
            indexMap[i] = index;

        vertexMap[index++] = vertex(loc.second);
    }

    // Reconstruct mesh
    const auto vc1 = int(vertexMap.size());
    const auto ic1 = int(mesh0.indices.size());
    std::vector<typename M::Vertex> vertices(vc1);
    std::vector<typename M::Index>  indices(ic1);

    // Vertices
    for (int i = 0; i < vc1; ++i)
        vertices[i] = vertexMap[i];

    // Indices
    for (int i = 0; i < ic1; ++i)
        indices[i] = indexMap[i];

    PTLOG(Info) << "verts: " << vc0 << " -> " << vc1;
    return M(vertices, indices);
}

template <typename M>
M smooth(const M& mesh0, int iterCount)
{
    if (iterCount > 0)
    {
        M mesh1(mesh0);
        //M mesh1(connect(mesh0));
        //return mesh1;

        const auto vc = int(mesh1.vertices.size());

        // Find neighbors
        Locations locations(vc / 8);
        for (int i = 0; i < vc; i += 3)
        {
            auto& l0 = locations[mesh1.vertices[i + 0].p];
            auto& l1 = locations[mesh1.vertices[i + 1].p];
            auto& l2 = locations[mesh1.vertices[i + 2].p];
            l0.insert(l0.end(), {i + 1, i + 2});
            l1.insert(l1.end(), {i + 0, i + 2});
            l2.insert(l2.end(), {i + 0, i + 1});
        }

        Neighbors neighbors(mesh1.vertices.size());

        #pragma omp parallel for
        for (int i = 0; i < vc; ++i)
            neighbors[i] = locations[mesh1.vertices[i].p];

        // Smooth iteratively
        const auto lambda = 0.5f;
        for (int c = 0; c < iterCount; ++c)
        {
            // Determine displacements
            Displacements displacements(vc, glm::vec3());

            #pragma omp parallel for
            for (int i = 0; i < vc; ++i)
            {
                const auto& v0 = mesh1.vertices[i];
                const auto& n  = neighbors[i];
                const auto nc  = int(n.size());
                if (nc)
                {
                    const auto w = 1.f / nc;
                    for (int j = 0; j < nc; ++j)
                    {
                        const auto& v1 = mesh1.vertices[n[j]];
                        displacements[i] += w * (v0.p - v1.p);
                    }
                }
            }
            // Apply displacements
            const float s = !(c % 2) ? lambda : -lambda;

            #pragma omp parallel for
            for(int i = 0; i < vc; ++i)
                mesh1.vertices[i].p += s * displacements[i];
        }

        // Recompute triangle normals and tangents
        #pragma omp parallel for
        for (int i = 0; i < vc; i += 3)
        {
            auto& va1 = mesh1.vertices[i + 0];
            auto& vb1 = mesh1.vertices[i + 1];
            auto& vc1 = mesh1.vertices[i + 2];
            auto n    = glm::normalize(glm::cross(vb1.p - va1.p, vc1.p - va1.p));
            auto t    = tangent({va1.p,  vb1.p,  vc1.p},
                                {va1.uv, vb1.uv, vc1.uv}, n);
            va1.n = n;
            vb1.n = n;
            vc1.n = n;
            va1.t = t;
            vb1.t = t;
            vc1.t = t;
        }
        return mesh1;
    }
    return mesh0;
}

template <typename M>
Vertices meshVertices(const M& mesh)
{
    const auto vertexCount = int(mesh.vertices.size());
    Vertices vertices;
    vertices.reserve(vertexCount);

    for (int i = 0; i < vertexCount; ++i)
        vertices.emplace_back(new Vertex(mesh.vertices[i].p, i));

    return vertices;
}

template <typename M>
Triangles meshTriangles(const M& mesh, Vertices& vertices)
{
    const auto triangleCount = mesh.triangleCount();
    Triangles triangles;
    triangles.reserve(triangleCount);

    for (int i = 0; i < triangleCount; ++i)
        triangles.emplace_back(new Triangle(vertices[mesh.indices[3 * i + 0]],
                                            vertices[mesh.indices[3 * i + 1]],
                                            vertices[mesh.indices[3 * i + 2]]));
    return triangles;
}

void reduce(Vertices& vertices, Triangles& triangles, int vertexCount);

template <typename M>
M reduce(const M& mesh0, int vertexCount)
{
    #if 0
    auto vertices  = meshVertices(mesh0);
    auto triangles = meshTriangles(mesh0, vertices);
    reduce(vertices, triangles, vertexCount);
    #endif

    return mesh0;
}

} // namespace ImageDeformer
} // namespace pt
