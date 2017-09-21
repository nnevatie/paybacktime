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

namespace
{

template<class T>
bool contains(const std::vector<T>& c, T t)
{
    return std::find(std::begin(c), std::end(c), t) != std::end(c);
}

template<class T>
int indexOf(const std::vector<T>& c, T t)
{
    return std::find(std::begin(c), std::end(c), t) - std::begin(c);
}

template<class T>
T& add(std::vector<T>& c, T t)
{
    c.push_back(t);
    return c.back();
}

template<class T>
T pop(std::vector<T>& c)
{
    auto val = std::move(c.back());
    c.pop_back();
    return val;
}

template<class T>
void addUnique(std::vector<T>& c, T t)
{
    if (!contains(c, t))
        c.push_back(t);
}

template<class T>
bool remove(std::vector<T>& c, T t)
{
    auto it = std::find(std::begin(c), std::end(c), t);
    if (it != end(c))
    {
        c.erase(it);
        return true;
    }
    return false;
}

float edgeCollapseCost(const Vertex* v0, const Vertex* v1)
{
    float edgeLength = glm::length(v0->pos - v1->pos);
    float curvature  = 0.f;

    std::vector<Triangle*> sides;
    for (const auto tri : v0->triangles)
        if (tri->contains(v1))
            sides.push_back(tri);

    for (const auto& tri : v0->triangles)
    {
        float curvatureMin = 1.f;
        for (const auto side : sides)
        {
            float dot    = glm::dot(tri->normal, side->normal);
            curvatureMin = std::min(curvatureMin, 0.5f * (1.f - dot));
        }
        curvature = std::max(curvature, curvatureMin);
    }
    return edgeLength * curvature;
}

bool updateEdgeCost(Vertex* vertex)
{
    if (!vertex->neighbors.empty())
    {
        float   costBest = std::numeric_limits<float>::max();
        Vertex* collapse = nullptr;

        for (const auto neighbor : vertex->neighbors)
        {
            float cost = edgeCollapseCost(vertex, neighbor);
            if (cost < costBest)
            {
                costBest = cost;
                collapse = neighbor;
            }
        }
        vertex->cost     = costBest;
        vertex->collapse = collapse;
        return true;
    }
    else
    {
        // No neighbors
        vertex->cost     = -1.f;
        vertex->collapse = nullptr;
        return false;
    }
}

void updateCollapseCosts(Vertices& vertices)
{
    for (auto& vertex : vertices)
        updateEdgeCost(vertex);
}

Vertex* costMin(const Vertices& vertices)
{
    // TODO: priority_queue
    Vertex* vertex = !vertices.empty() ? vertices.front() : nullptr;
    for (auto v : vertices)
        if (v->cost < vertex->cost)
            vertex = v;

    return vertex;
}

void collapseEdge(Vertices& vertices, Triangles& triangles, Vertex* v0, Vertex* v1)
{
    if (!v1)
    {
        remove(vertices, v0);
        delete v0;
        return;
    }

    Vertices neighbors = v0->neighbors;

    for (int i = int(v0->triangles.size() - 1); i >= 0; --i)
    {
        auto triangle = v0->triangles[i];
        if (triangle->contains(v1))
        {
            remove(triangles, triangle);
            delete triangle;
        }
    }

    for (int i = int(v0->triangles.size() - 1); i >= 0; --i)
        v0->triangles[i]->replace(v0, v1);

    remove(vertices, v0);
    delete v0;

    for (auto neighbor : neighbors)
        updateEdgeCost(neighbor);
}

} // namespace

Triangle::Triangle(Vertex* v0, Vertex* v1, Vertex* v2) :
    vertices {v0, v1, v2}
{
    updateNormal();
    for(int i = 0; i < 3; ++i)
    {
        vertices[i]->triangles.push_back(this);
        for(int j = 0; j < 3; ++j)
            if(i != j)
                addUnique(vertices[i]->neighbors, vertices[j]);
    }
}

Triangle::~Triangle()
{
    for(int i = 0; i < 3; ++i)
        if(vertices[i])
            remove(vertices[i]->triangles, this);

    for (int i0 = 0; i0 < 3; ++i0)
    {
        int i1 = (i0 + 1) % 3;
        if(vertices[i0] && vertices[i1])
        {
            vertices[i0]->removeNonNeighbor(vertices[i1]);
            vertices[i1]->removeNonNeighbor(vertices[i0]);
        }
    }
}

bool Triangle::contains(const Vertex* v) const
{
    return v == vertices[0] || v == vertices[1] || v == vertices[2];
}

bool Triangle::updateNormal()
{
    auto v0 = vertices[0]->pos;
    auto v1 = vertices[1]->pos;
    auto v2 = vertices[2]->pos;
    normal  = glm::cross(v1 - v0, v2 - v1);
    if(glm::length(normal) > 0.f)
    {
        normal = normalize(normal);
        return true;
    }
    return false;
}

Triangle& Triangle::replace(Vertex* v0, Vertex* v1)
{
    if(v0 == vertices[0])
        vertices[0] = v1;
    else
    if(v0 == vertices[1])
        vertices[1] = v1;
    else
        vertices[2] = v1;

    remove(v0->triangles, this);
    v1->triangles.push_back(this);

    for (int i = 0; i < 3; ++i)
    {
        v0->removeNonNeighbor(vertices[i]);
        vertices[i]->removeNonNeighbor(v0);
    }
    for (int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
            if(i != j)
                addUnique(vertices[i]->neighbors, vertices[j]);

    updateNormal();
    return *this;
}

Vertex::Vertex(const glm::vec3& pos, int id) :
    pos(pos), id(id), cost(0.f), collapse(nullptr)
{}

Vertex::~Vertex()
{
    while(neighbors.size())
    {
        remove(neighbors[0]->neighbors, this);
        remove(neighbors, neighbors[0]);
    }
}

bool Vertex::removeNonNeighbor(Vertex* v)
{
    if(contains(neighbors, v))
    {
        for (const auto& tri : triangles)
            if (tri->contains(v)) return false;

        remove(neighbors, v);
        return true;
    }
    return false;
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

void decimate(Vertices& vertices, Triangles& triangles, int triangleCount)
{
    updateCollapseCosts(vertices);
    while (int(triangles.size()) > triangleCount)
    {
        auto v = costMin(vertices);
        collapseEdge(vertices, triangles, v, v->collapse);
    }
}

Mesh_P_N_T_UV decimate(const Mesh_P_N_T_UV& mesh0, int triangleCount)
{
    auto mesh1 = connect(mesh0);
    PTTIMEU("decimate", boost::milli);

    auto v = meshVertices(mesh1.first);
    auto t = meshTriangles(mesh1.first, v);
    decimate(v, t, triangleCount);

    const int tc = int(t.size());

    Mesh_P_N_T_UV mesh2;
    mesh2.vertices.resize(tc * 3);
    mesh2.indices.resize(tc * 3);

    auto bestUv = [&](const glm::vec3& tn, const Indices& indices)
    {
        int bi   = -1;
        float bd = -2.f;
        for (const auto i : indices)
        {
            const auto vn = mesh0.vertices[i].n;
            const auto d  = glm::dot(tn, vn);
            if (d > bd)
            {
                bi = i;
                bd = d;
            }
        }
        return mesh0.vertices[bi].uv;
    };

    for (int ti = 0; ti < tc; ++ti)
    {
        auto& tri = t[ti];
        for (int vi = 0; vi < 3; ++vi)
        {
            const auto& p = tri->vertices[vi]->pos;
            const auto& v = mesh0.vertices[mesh1.second[p].front()];
            const auto uv = bestUv(tri->normal, mesh1.second[p]);
            mesh2.vertices[3 * ti + vi] = {p, v.n, v.t, uv};
            mesh2.indices[3 * ti + vi] = 3 * ti + vi;
        }
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
        Neighbors neighbors(vc);
        Locations locations(vc / 8);

        PTLOG(Info) << "verts: " << vc;
        for (int i = 0; i < vc; i += 3)
        {
            auto& l0 = locations[mesh1.vertices[i + 0].p.xyz];
            auto& l1 = locations[mesh1.vertices[i + 1].p.xyz];
            auto& l2 = locations[mesh1.vertices[i + 2].p.xyz];
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
                    d[i][0] += v0.p.xyz - v1.p.xyz;
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
