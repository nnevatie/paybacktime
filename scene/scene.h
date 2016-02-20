#pragma once

#include <glm/vec3.hpp>

#include "object.h"

namespace pt
{

struct Intersection
{
    glm::vec3 pos;
    Object    object;
};

struct Scene
{
    Scene();

    Intersection intersect(const glm::vec3& origin,
                           const glm::vec3& ray) const;

    Scene& render();
};

} // namespace pt
