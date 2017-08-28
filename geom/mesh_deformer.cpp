#include "mesh_deformer.h"

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

bool Triangle::contains(Vertex* v) const
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

Vertex::Vertex(const glm::vec3& pos, int id) :
    pos(pos), id(id), cost(0.f), collapse(nullptr)
{}

Vertex::~Vertex()
{
    assert(triangles.empty());
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

} // namespace ImageDeformer
} // namespace pt
