#pragma once

#include "common/log.h"
#include "mesh.h"

namespace pt
{
namespace SnMesher
{

struct EdgeTable
{
    int cubeEdges[24] = {0};
    int edges[256]    = {0};

    EdgeTable()
    {
        // Initialize the cube edges
        // The vertex number of each cube
        int k = 0;
        for(int i = 0; i < 8; ++i)
            for(int j = 1; j <= 4; j <<= 1)
            {
                int p = i ^ j;
                if(i <= p)
                {
                    cubeEdges[k++] = i;
                    cubeEdges[k++] = p;
                }
            }

        // Initialize the intersection table.
        // This is a 2^(cube configuration) -> 2^(edge configuration) map
        // There is one entry for each possible cube configuration
        // and the output is a 12-bit vector enumerating all edges crossing
        // the 0-level.
        for(int i = 0; i < 256; ++i)
        {
            int em = 0;
            for(int j = 0; j < 24; j += 2)
            {
                int a = !(i & (1 << cubeEdges[j]));
                int b = !(i & (1 << cubeEdges[j + 1]));
                em   |= a != b ? (1 << (j >> 1)) : 0;
            }
            edges[i] = em;
        }
    }
};

static EdgeTable edgeTable;

inline int dominantAxis(const glm::vec3& v)
{
    const glm::vec3 axes[] =
    {
        glm::vec3( 0,  0, +1),
        glm::vec3( 0,  0, -1),
        glm::vec3(-1,  0,  0),
        glm::vec3(+1,  0,  0),
        glm::vec3( 0, +1,  0),
        glm::vec3( 0, -1,  0)
    };

    int axisIndex = -1;
    float score   = 0.f;
    for (int i = 0; i < 6; ++i)
    {
        const auto d = glm::dot(axes[i], v);
        if (d >= score)
        {
            axisIndex = i;
            score     = d;
        }
    }
    return axisIndex;
}

template <typename V>
Mesh_P_N_T_UV mesh(const V& vol, const RectCube<float>& uvCube, float scale = 1.f)
{
    const std::array<int, 3>& dims = {vol.width + 2, vol.height + 2, vol.depth + 2};

    int x[3]      = {0, 0, 0};
    int r[3]      = {1, (dims[0] + 1), (dims[0] + 1) * (dims[1] + 1)};
    float grid[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    std::vector<int> buffer(r[2] * 2, 0);

    std::vector<glm::vec3> vertices;
    vertices.reserve(buffer.size());

    Mesh_P_N_T_UV mesh;
    mesh.vertices.reserve(buffer.size() * 16);
    mesh.indices.reserve(buffer.size()  * 4);

    //March over the voxel grid
    int bufNo = 1;
    for (x[2] = 0; x[2] < dims[2] - 1; ++x[2], bufNo ^= 1, r[2] = -r[2])
    {
        int m = 1 + (dims[0] + 1) * (1 + bufNo * (dims[1] + 1));

        for (x[1] = 0; x[1] < dims[1] - 1; ++x[1], m += 2)
            for (x[0] = 0; x[0] < dims[0] - 1; ++x[0], ++m)
            {
                int mask = 0, g = 0;
                for (int k = 0; k < 2; ++k)
                for (int j = 0; j < 2; ++j)
                for (int i = 0; i < 2; ++i, ++g)
                {
                    int p   = vol(x[0] + i - 1, x[1] + j - 1, x[2] + k - 1);
                    grid[g] = p;
                    mask   |= (p > 0) ? (1 << g) : 0;
                }

                // Early termination if cell does not intersect boundary
                if (mask == 0 || mask == 0xff)
                    continue;

                int edgeMask  = edgeTable.edges[mask];
                int edgeCount = 0;
                float v[3]    = {0, 0, 0};

                // For every edge of the cube
                for (int i = 0; i < 12; ++i)
                {
                    // Use edge mask to check if it is crossed
                    if (!(edgeMask & (1 << i)))
                        continue;

                    //If it did, increment number of edge crossings
                    ++edgeCount;

                    int e0   = edgeTable.cubeEdges[ i << 1     ];
                    int e1   = edgeTable.cubeEdges[(i << 1) + 1];
                    float g0 = grid[e0];
                    float g1 = grid[e1];
                    float t  = g0 - g1;

                    if (std::abs(t) > 0)
                        t = g0 / t;
                    else
                        continue;

                    //Interpolate vertices and add up intersections
                    for (int j = 0, k = 1; j < 3; ++j, k <<= 1)
                    {
                        int a = e0 & k;
                        int b = e1 & k;
                        if (a != b)
                            v[j] += a ? 1 - t : t;
                        else
                            v[j] += a ? 1 : 0;
                    }
                }

                // Average the edge intersections and add them to coordinate
                float s = 1.0f / edgeCount;
                for (int i = 0; i < 3; ++i)
                    v[i] = (x[i] + s * v[i]);

                // Add vertex to buffer,
                // store pointer to vertex index in buffer
                buffer[m] = vertices.size();
                vertices.push_back({glm::vec3(v[0], v[1], v[2]) * scale});

                // Now we need to add faces together,
                // to do this we just loop over 3 basis components
                for (int i = 0; i < 3; ++i)
                {
                    //The first three entries of the edgeMask count
                    // the crossings along the edge
                    if (!(edgeMask & (1 << i)))
                        continue;

                    // i     = axes we are point along,
                    // iu/iv = orthogonal axes
                    int iu = (i + 1) % 3;
                    int iv = (i + 2) % 3;

                    // If we are on a boundary, skip it
                    if (x[iu] == 0 || x[iv] == 0)
                        continue;

                    // Otherwise, look up adjacent edges in buffer
                    int du = r[iu];
                    int dv = r[iv];

                    const auto& va = vertices[buffer[m]];
                    const auto& vb = vertices[buffer[m - du]];
                    const auto& vc = vertices[buffer[m - du - dv]];
                    const auto& vd = vertices[buffer[m - dv]];
                    const auto ns  = mask & 1 ? 1 : -1;
                    const auto n0  = ns * glm::normalize(
                                          glm::cross(vb - va, vc - va));
                    const auto n1  = ns * glm::normalize(
                                          glm::cross(vd - vc, va - vc));

                    // Remember to flip orientation
                    // depending on the sign of the corner
                    const Mesh_P_N_UV::Index ib = mesh.vertices.size();
                    if (mask & 1)
                        mesh.vertices.insert(mesh.vertices.begin(),
                                           {{va, n0},
                                            {vb, n0},
                                            {vc, n0},
                                            {vc, n1},
                                            {vd, n1},
                                            {va, n1}});
                    else
                        mesh.vertices.insert(mesh.vertices.begin(),
                                           {{vc, n0},
                                            {vb, n0},
                                            {va, n0},
                                            {va, n1},
                                            {vd, n1},
                                            {vc, n1}});

                    mesh.indices.insert(mesh.indices.end(),
                                       {ib + 0, ib + 1, ib + 2,
                                        ib + 3, ib + 4, ib + 5});
                }
            }
    }
    return mesh;
}

} // namespace SnMesher
} // namespace pt

