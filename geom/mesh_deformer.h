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
using Neighbors     = std::vector<Indices>;
using Displacements = std::vector<std::array<glm::vec3, 3>>;

template <typename M>
M connect(const M& mesh0)
{
    PTTIMEU("connect", boost::milli);
    const auto vc0 = int(mesh0.vertices.size());

    Locations locations(vc0 / 8);
    for (int i = 0; i < vc0; ++i)
        locations[mesh0.vertices[i].p].emplace_back(i);

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
            uv  = mesh0.vertices[i].uv;
        }
        const float c = float(indices.size());
        return typename M::Vertex {p / c, n / c, t / c, uv};
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

    std::sort(std::begin(indexMap), std::end(indexMap));
    auto last = std::unique(std::begin(indexMap), std::end(indexMap));
    indexMap.erase(last, std::end(indexMap));
    const auto uic1 = indexMap.size();

    PTLOG(Info) << "verts: " << vc0 << " -> " << vc1;
    PTLOG(Info) << "indices: " << ic1 << ", uniq: " << uic1;
    return M(vertices, indices);
}

template <typename M>
M smooth(const M& mesh0, int iterCount)
{
    if (iterCount > 0)
    {
        PTTIMEU("smooth", boost::milli);
        M mesh1(mesh0);
        const auto vc = int(mesh1.vertices.size());
        const auto tc = int(mesh1.triangleCount());

        // Find neighbors
        Neighbors neighbors(vc);
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

        #pragma omp parallel for
        for (int i = 0; i < vc; ++i)
            neighbors[i] = locations[mesh1.vertices[i].p];

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
                    d[i][0] += (v0.p - v1.p);
                    d[i][1] += (v0.n - v1.n);
                    d[i][2] += (v0.t - v1.t);
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

Mesh_P_N_T_UV reduce(const Mesh_P_N_T_UV& mesh0, int vertexCount);

} // namespace ImageDeformer
} // namespace pt
