#pragma once

#include <array>
#include <vector>
#include <iostream>

#include <glm/vec3.hpp>

namespace hc
{

typedef std::array<glm::vec3, 8>        Box;
typedef std::pair<glm::vec3, glm::vec3> BoundingBox;

struct Geometry
{
    typedef glm::vec3 Vertex;
    typedef uint16_t  Index;

    std::vector<Vertex> vertices;
    std::vector<Index>  indices;

    Geometry() {}

    Geometry(const std::vector<Vertex>& vertices,
             const std::vector<Index>&  indices) :
        vertices(vertices), indices(indices) {}

    friend std::ostream& operator<<(std::ostream& out, const Geometry& geometry)
    {
        out << "Geometry[vertices: " << geometry.vertices.size()
            << ", triangles: " << geometry.indices.size() / 3 << "]";

        return out;
    }
};

} // namespace
