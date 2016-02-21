#pragma once

#include <memory>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "gfx/geometry.h"
#include "object.h"

namespace pt
{

struct Scene
{
    struct Item
    {
        Object    obj;
        glm::vec3 pos;
    };

    Scene();

    Scene& add(const Item& item);

    Item intersect(const glm::vec3& origin,
                   const glm::vec3& ray) const;

    gfx::Geometry::Instances geometryInstances() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
