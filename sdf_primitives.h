#pragma once

#include <glm/vec3.hpp>

namespace hc
{
namespace sdf
{

// Sphere with radius r
struct Sphere
{
    Sphere(float r);
    float operator()(const glm::vec3& p) const;

    float r;
};

// Box with diagonal b
struct Box
{
    Box(const glm::vec3& b);
    float operator()(const glm::vec3& p) const;

    glm::vec3 b;
};

} // namespace sdf
} // namespace hc

