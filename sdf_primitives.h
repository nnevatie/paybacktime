#pragma once

#include <glm/vec3.hpp>

#include "mesh.h"

namespace hc
{
namespace sdf
{

// Sphere with radius r
struct Sphere
{
    explicit Sphere(float r);
    float operator()(const glm::vec3& p) const;
    BoundingBox bounds() const;

    float r;
};

// Box with diagonal b
struct Box
{
    explicit Box(const glm::vec3& b);
    float operator()(const glm::vec3& p) const;
    BoundingBox bounds() const;

    glm::vec3 b;
};

} // namespace sdf
} // namespace hc

