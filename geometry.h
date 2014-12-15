#pragma once

#include <vector>

#include <glm/vec3.hpp>

namespace hc
{

struct Geometry
{
    typedef glm::vec3 Vertex;
    typedef unsigned short Index;

    std::vector<Vertex> vertices;
    std::vector<Index>  indices;
};

} // namespace
