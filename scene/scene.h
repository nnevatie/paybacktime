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
#include "character.h"

namespace pt
{

struct ObjectItem
{
    Object         obj;
    TransformTrRot trRot;

    Box bounds() const;
    bool operator==(const ObjectItem& other) const;
    bool operator!=(const ObjectItem& other) const;
};
typedef std::vector<ObjectItem>           ObjectItems;
typedef std::pair<glm::vec3, ObjectItems> Intersection;

struct Scene
{
    enum class GeometryType
    {
        Any,
        Opaque,
        Transparent
    };

    Scene();

    Box bounds() const;

    bool contains(const ObjectItem& item) const;

    Scene& add(const ObjectItem& item);
    bool remove(const ObjectItem& item);

    Intersection intersect(const Ray& ray) const;

    gfx::Geometry::Instances objectGeometry(
        GeometryType type = GeometryType::Any) const;

    gl::Texture* lightmap() const;
    gl::Texture* incidence() const;

    Scene& updateLightmap();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

bool containsItem(const ObjectItems& items, const ObjectItem& item);
bool containsObject(const ObjectItems& items, const Object& object);

} // namespace pt
