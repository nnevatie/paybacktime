#pragma once

#include <memory>
#include <vector>
#include <utility>

#include <glm/vec3.hpp>

#include "geom/box.h"
#include "geom/ray.h"
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

        Box bounds() const
        {
            return {pos, obj.model().dimensions()};
        }
        operator==(const Item& other) const
        {
            return obj == other.obj && pos == other.pos;
        }
        operator!=(const Item& other) const
        {
            return !operator==(other);
        }
    };
    typedef std::vector<Item>           Items;
    typedef std::pair<glm::vec3, Items> Intersection;

    Scene();

    bool contains(const Item& item) const;

    Scene& add(const Item& item);

    Intersection intersect(const Ray& ray) const;

    gfx::Geometry::Instances geometryInstances() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

bool containsItem(const Scene::Items& items, const Scene::Item& item);
bool containsObject(const Scene::Items& items, const Object& object);

} // namespace pt
