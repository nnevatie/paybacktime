#pragma once

#include <vector>

#include <glm/vec3.hpp>

namespace hc
{

typedef std::pair<glm::vec3, glm::vec3> BoundingBox;

struct Geometry
{
    typedef glm::vec3 Vertex;
    typedef unsigned short Index;

    std::vector<Vertex> vertices;
    std::vector<Index>  indices;
};

} // namespace
