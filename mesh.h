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

struct Mesh
{
    struct Vertex
    {
        glm::vec3 p;
        glm::vec3 n;
        glm::vec2 uv;

        Vertex() {}
        Vertex(const glm::vec3& p) : p(p) {}
        Vertex(const glm::vec3& p, const glm::vec3& n) : p(p), n(n) {}
        Vertex(const glm::vec3& p, const glm::vec3& n, const glm::vec2& uv) :
            p(p), n(n), uv(uv) {}

        Vertex(float x,  float y,  float z) : p(x, y, z) {}
        Vertex(float x,  float y,  float z,
               float nx, float ny, float nz,
               float u,  float v) : p(x, y, z), n(nx, ny, nz), uv(u, v) {}
    };

    typedef uint32_t Index;

    std::vector<Vertex> vertices;
    std::vector<Index>  indices;

    Mesh() {}

    Mesh(const std::vector<Vertex>& vertices,
         const std::vector<Index>& indices) :
         vertices(vertices), indices(indices) {}

    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
    {
        out << "Mesh[vertices: " << mesh.vertices.size()
            << ", triangles: " << mesh.indices.size() / 3 << "]";

        return out;
    }
};

Mesh squareMesh(float halfWidth = 1.f);
Mesh rectMesh(float halfWidth = 1.f, float halfHeight = 1.f);

} // namespace
