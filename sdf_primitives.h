#pragma once

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

namespace hc
{
namespace sdf
{

// Sphere of radius r
struct Sphere
{
    Sphere(float r) : r(r) {}

    float operator()(const glm::vec3& p) const
    {
        return glm::length(p) - r;
    }

    float r;
};

} // namespace sdf
} // namespace hc

