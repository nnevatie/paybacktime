#include "mesh_deformer.h"

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

#include "platform/clock.h"
#include "common/log.h"

#include "mesh_common.h"
#include "mesh_simplifier.h"

namespace pt
{
namespace MeshDeformer
{
// Types
using Indices       = std::vector<int>;
using PosNormal     = std::pair<glm::vec3, glm::vec3>;
using Locations     = std::unordered_map<glm::vec3, Indices>;
using Neighbors     = std::vector<Indices>;
using Displacements = std::vector<std::array<glm::vec3, 3>>;
using UniqueMesh    = std::pair<Mesh, Locations>;

template <typename M>
MeshSimplifier::Simplifier::Verts meshVertices(const M& mesh)
{
    const auto vertexCount = int(mesh.vertices.size());
    MeshSimplifier::Simplifier::Verts vertices;
    vertices.reserve(vertexCount);

    for (int i = 0; i < vertexCount; ++i)
        vertices.emplace_back(mesh.vertices[i].p);

    return vertices;
}

template <typename M>
MeshSimplifier::Simplifier::Tris meshTriangles(const M& mesh)
{
    const auto triangleCount = mesh.triangleCount();
    MeshSimplifier::Simplifier::Tris triangles;
    triangles.reserve(triangleCount);

    for (int i = 0; i < triangleCount; ++i)
    {
        auto i0 = mesh.indices[3 * i + 0];
        auto i1 = mesh.indices[3 * i + 1];
        auto i2 = mesh.indices[3 * i + 2];
        auto p0 = mesh.vertices[i0].p;
        auto p1 = mesh.vertices[i1].p;
        auto p2 = mesh.vertices[i2].p;
        auto n  = glm::normalize(glm::cross(p1 - p0, p2 - p1));
        triangles.emplace_back(i0, i1, i2, n);
    }
    return triangles;
}

namespace
{

Neighbors vertexNeighbors(const Mesh& mesh)
{
    const auto& v = mesh.vertices;
    const int vc  = int(v.size());

    Locations locations(vc / 8);
    for (int i = 0; i < vc; i += 3)
    {
        auto& l0 = locations[v[i + 0].p];
        auto& l1 = locations[v[i + 1].p];
        auto& l2 = locations[v[i + 2].p];
        l0.insert(l0.end(), {i + 1, i + 2});
        l1.insert(l1.end(), {i + 2, i + 0});
        l2.insert(l2.end(), {i + 0, i + 1});
    }

    Neighbors neighbors(vc);
    #pragma omp parallel for
    for (int i = 0; i < vc; ++i)
        neighbors[i] = locations[v[i].p];

    return neighbors;
}

UniqueMesh connect(const Mesh_P_N_T_UV& mesh0)
{
    PTTIMEU("connect", boost::milli);
    const auto vc0 = int(mesh0.vertices.size());

    Locations locations(vc0 / 8);
    for (int i = 0; i < vc0; ++i)
        locations[mesh0.vertices[i].p].emplace_back(i);

    std::vector<int> indexMap(vc0);
    std::vector<Mesh::Vertex> vertexMap(locations.size());

    int index = 0;
    for (const auto& loc : locations)
    {
        for (const auto i : loc.second)
            indexMap[i] = index;

        vertexMap[index++] = mesh0.vertices[loc.second.front()];
    }

    // Reconstruct mesh
    const auto vc1 = int(vertexMap.size());
    const auto ic1 = int(mesh0.indices.size());
    std::vector<Mesh::Vertex> vertices(vc1);
    std::vector<Mesh::Index>  indices(ic1);

    // Vertices
    for (int i = 0; i < vc1; ++i)
        vertices[i] = vertexMap[i];

    // Indices
    for (int i = 0; i < ic1; ++i)
        indices[i] = indexMap[i];

    return {Mesh(vertices, indices), locations};
}

} // namespace

Mesh_P_N_T_UV decimate(const Mesh_P_N_T_UV& mesh0,
                       const RectCube<float>& uvCube,
                       const glm::vec3& size)
{
    auto mesh1 = connect(mesh0);
    PTTIMEU("decimate", boost::milli);

    auto v = meshVertices(mesh1.first);
    auto t = meshTriangles(mesh1.first);

    MeshSimplifier::Simplifier simplifier(t, v);
    simplifier.simplify(5, 0.01f, 2.f);

    const int vc0 = int(v.size());
    const int tc0 = int(t.size());

    PTLOG(Info) << mesh1.first.vertices.size() << " / "
                << mesh1.first.triangleCount() << " -> " << vc0 << " / " << tc0;

    Mesh_P_N_T_UV mesh2;
    mesh2.vertices.resize(tc0 * 3);
    mesh2.indices.resize(tc0 * 3);

    // Reconstruct non-vertex-sharing triangles
    #pragma omp parallel for
    for (int ti = 0; ti < tc0; ++ti)
    {
        auto& tri = t[ti];
        auto n    = glm::normalize(glm::cross(v[tri.v[1]].p - v[tri.v[0]].p,
                                              v[tri.v[2]].p - v[tri.v[1]].p));
        auto t    = glm::zero<glm::vec3>();
        for (int vi = 0; vi < 3; ++vi)
        {
            const auto& p               = v[tri.v[vi]].p;
            const auto uv               = glm::vec2(0, 0);
            mesh2.vertices[3 * ti + vi] = {p, n, t, uv};
            mesh2.indices[3 * ti + vi]  = 3 * ti + vi;
        }
    }

    // Reconstruct normals and UVs
    const int axisUV[6][2] =
    {
        {2, 1}, {2, 1},
        {0, 2}, {0, 2},
        {0, 1}, {0, 1},
    };
    const int axisSide[6] = {2, 3, 5, 4, 1, 0};
    const auto neighbors  = vertexNeighbors(mesh2);
    const auto vc1        = int(mesh2.vertices.size());
    const auto tc1        = int(mesh2.triangleCount());
    for (int i = 0; i < vc1; ++i)
    {
        auto&       v0  = mesh2.vertices[i];
        const auto& idx = neighbors[i];
        const auto  ic  = int(idx.size());
        glm::vec3   n   = glm::zero<glm::vec3>();
        for (int j = 0; j < ic; j += 2)
        {
            const auto& v1 = mesh2.vertices[idx[j + 0]];
            const auto& v2 = mesh2.vertices[idx[j + 1]];
            const auto  e0 = v1.p - v0.p,
                        e1 = v2.p - v1.p;
            const auto  l0 = glm::length(e0),
                        l1 = glm::length(e1);
            const auto  a  = glm::angle(e0 / l0, e1 / l1);
            const auto  ar = 0.5f * l0 * l1 * glm::sin(a);
            n             += a * ar * glm::normalize(glm::cross(e0, e1));
        }
        const auto axis = dominantAxis(v0.n);
        const auto uvn  = glm::vec2(v0.p[axisUV[axis][0]] / size[axisUV[axis][0]],
                                    v0.p[axisUV[axis][1]] / size[axisUV[axis][1]]);
        const auto uv   = uvCube[axisSide[axis]].point(uvn.x, uvn.y);
        v0.n            = glm::normalize(n);
        v0.uv           = uv;
    }
    // Reconstruct tangents
    for (int i = 0; i < tc1; ++i)
    {
        auto& v0 = mesh2.vertices[i * 3 + 0];
        auto& v1 = mesh2.vertices[i * 3 + 1];
        auto& v2 = mesh2.vertices[i * 3 + 2];
        const auto n = glm::normalize(glm::cross(v1.p - v0.p, v2.p - v1.p));
        const auto t = tangent({v0.p, v1.p, v2.p},
                               {v0.uv, v1.uv, v2.uv}, n);
        v0.t = t;
        v1.t = t;
        v2.t = t;
    }
    return mesh2;
}

Mesh smooth(const Mesh& mesh0, int iterCount)
{
    if (iterCount > 0)
    {
        PTTIMEU("smooth", boost::milli);
        Mesh mesh1(mesh0);
        const auto vc = int(mesh1.vertices.size());

        // Find neighbors
        const auto neighbors = vertexNeighbors(mesh1);

        // Smooth iteratively with strength lambda
        const auto lambda = 0.5f;

        Displacements d(vc);
        std::array<glm::vec3, 3> zero;
        zero.fill(glm::zero<glm::vec3>());

        for (int c = 0; c < iterCount; ++c)
        {
            // Determine displacements
            std::fill(std::begin(d), std::end(d), zero);

            #pragma omp parallel for
            for (int i = 0; i < vc; ++i)
            {
                const auto& v0 = mesh1.vertices[i];
                const auto& n  = neighbors[i];
                const auto nc  = int(n.size());
                for (int j = 0; j < nc; ++j)
                {
                    const auto& v1 = mesh1.vertices[n[j]];
                    d[i][0] += v0.p - v1.p;
                    d[i][1] += v0.n - v1.n;
                    d[i][2] += v0.t - v1.t;
                }
            }
            // Apply displacements
            const float s = !(c % 2) ? lambda : -lambda;

            #pragma omp parallel for
            for(int i = 0; i < vc; ++i)
            {
                const auto w = 1.f / neighbors[i].size();
                auto& v1     = mesh1.vertices[i];
                v1.p += s * w * d[i][0];
                v1.n += s * w * d[i][1];
                v1.t += s * w * d[i][2];
            }
        }
        return mesh1;
    }
    return mesh0;
}

} // namespace ImageDeformer
} // namespace pt
