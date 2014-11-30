#pragma once

#include <vector>

#include <glm/vec3.hpp>

namespace hc
{

struct Geometry
{
    std::vector<glm::vec3>      vertices;
    std::vector<unsigned short> indices;
};

} // namespace
