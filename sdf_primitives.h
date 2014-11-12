#pragma once

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

namespace hc
{
namespace sdf
{

// Sphere with radius r
struct Sphere
{
    Sphere(float r) : r(r) {}

    float operator()(const glm::vec3& p) const
    {
        return glm::length(p) - r;
    }

    float r;
};

// Box with diagonal b
struct Box
{
    Box(const glm::vec3& b) : b(b) {}

    float operator()(const glm::vec3& p) const
    {
        glm::vec3 d = glm::abs(p) - b;
        return glm::min(glm::max(d.x, glm::max(d.y, d.z)), 0.f) +
               glm::length(glm::max(d, 0.f));
    }

    glm::vec3 b;
};

} // namespace sdf
} // namespace hc

