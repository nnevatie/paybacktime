#pragma once

#include <memory>
#include <vector>
#include <utility>

#include <glm/vec3.hpp>

#include "geom/box.h"
#include "geom/ray.h"
#include "geom/transform.h"

#include "img/image.h"
#include "gfx/geometry.h"
#include "gl/texture.h"

#include "object.h"

namespace pt
{

struct Scene
{
    struct Item
    {
        Object         obj;
        TransformTrRot trRot;

        Box bounds() const;
        bool operator==(const Item& other) const;
        bool operator!=(const Item& other) const;
    };

    typedef std::vector<Item>           Items;
    typedef std::pair<glm::vec3, Items> Intersection;

    Scene();

    Box bounds() const;

    bool contains(const Item& item) const;

    Scene& add(const Item& item);
    bool remove(const Item& item);

    Intersection intersect(const Ray& ray) const;

    gfx::Geometry::Instances geometryInstances() const;

    gl::Texture* lightmap() const;
    Scene& updateLightmap();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

bool containsItem(const Scene::Items& items, const Scene::Item& item);
bool containsObject(const Scene::Items& items, const Object& object);

} // namespace pt
