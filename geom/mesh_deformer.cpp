#include "mesh_deformer.h"

#include <boost/thread.hpp>
namespace std {using thread = boost::thread;}
#include <igl/decimate.h>

namespace pt
{
namespace MeshDeformer
{

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

    for (auto triangle : v0->triangles)
        if (triangle->contains(v1))
        {
            remove(triangles, triangle);
            delete triangle;
        }

    for (auto triangle : v0->triangles)
        triangle->replace(v0, v1);

    Vertices neighbors = v0->neighbors;

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

void reduce(Vertices& vertices, Triangles& triangles, int vertexCount)
{
    updateCollapseCosts(vertices);

    while (int(vertices.size()) > vertexCount)
    {
        auto v = costMin(vertices);
        collapseEdge(vertices, triangles, v, v->collapse);
    }
}

} // namespace ImageDeformer
} // namespace pt
