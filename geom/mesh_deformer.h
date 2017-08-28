#pragma once

#include <vector>
#include <unordered_map>

#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

#include "common/log.h"

#include "mesh.h"

namespace pt
{
namespace MeshDeformer
{

struct Triangle;
struct Vertex;

struct Triangle
{
    Vertex*   vertices[3];
    glm::vec3 normal;

    Triangle(Vertex* v0, Vertex* v1, Vertex* v2);
    ~Triangle();

    bool      contains(Vertex* v) const;
    bool      updateNormal();
    Triangle& replace(Vertex* v0, Vertex* v1);
};
using Triangles = std::vector<Triangle>;

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
using Vertices = std::vector<Vertex>;

template <typename M>
M smooth(const M& mesh0, int count)
{
    M mesh1(mesh0);
    if (count > 0)
    {
        // Types
        using Indices       = std::vector<int>;
        using Locations     = std::unordered_map<glm::vec3, Indices>;
        using Neighbors     = std::vector<Indices>;
        using Displacements = std::vector<glm::vec3>;

        // Vertex/index counts
        const auto vc = int(mesh1.vertices.size());

        // Find neighbors
        Locations locations(vc / 4);
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
        for (int c = 0; c < count; ++c)
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
    }
    return mesh1;
}

template <typename M>
Vertices meshVertices(const M& mesh)
{
    const auto vertexCount = int(mesh.vertices.size());

    Vertices vertices;
    vertices.reserve(vertexCount);

    for (int i = 0; i < vertexCount; ++i)
        vertices.push_back({mesh.vertices[i].p, i});

    return vertices;
}

template <typename M>
Triangles meshTriangles(const M& mesh, Vertices& vertices)
{
    const auto triangleCount = mesh.triangleCount();

    Triangles triangles;
    triangles.reserve(triangleCount);

    for (int i = 0; i < triangleCount; ++i)
        triangles.push_back({&vertices[mesh.indices[3 * i + 0]],
                             &vertices[mesh.indices[3 * i + 1]],
                             &vertices[mesh.indices[3 * i + 2]]});
    return triangles;
}

template <typename M>
M reduce(const M& mesh0, int count)
{
    auto vertices  = meshVertices(mesh0);
    auto triangles = meshTriangles(mesh0, vertices);
    return mesh0;
}

} // namespace ImageDeformer
} // namespace pt
