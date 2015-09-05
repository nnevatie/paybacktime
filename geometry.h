#pragma once

#include <array>
#include <vector>
#include <iostream>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace hc
{

typedef std::array<glm::vec3, 8>        Box;
typedef std::pair<glm::vec3, glm::vec3> BoundingBox;

struct Geometry
{
    struct Vertex
    {
        glm::vec3 p;
        glm::vec3 n;
        glm::vec2 uv;

        Vertex() {}
        Vertex(const glm::vec3& p) : p(p) {}
        Vertex(const glm::vec3& p, const glm::vec3& n) : p(p), n(n) {}
        Vertex(float x, float y, float z) : p(x, y, z) {}
        Vertex(float x,  float y,  float z,
               float nx, float ny, float nz,
               float u,  float v) : p(x, y, z), n(nx, ny, nz), uv(u, v) {}
    };

    typedef uint32_t Index;

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

Geometry squareGeometry(float halfWidth = 1.f);
Geometry rectGeometry(float halfWidth = 1.f, float halfHeight = 1.f);

} // namespace
