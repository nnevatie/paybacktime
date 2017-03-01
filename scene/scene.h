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

#include "scene_item.h"

namespace pt
{

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
    glm::ivec3 cellResolution() const;

    Scene& setHorizon(const Horizon& horizon);

    Scene& add(const ObjectItem& item);
    bool remove(const ObjectItem& item);

    Scene& add(const CharacterItem& item);

    ObjectItems intersect(const ObjectItem& item) const;
    Intersection intersect(const Ray& ray) const;

    gfx::Geometry::Instances objectGeometry(
        GeometryType type = GeometryType::Any) const;

    gfx::Geometry::Instances characterGeometry() const;

    gl::Texture* lightmap() const;
    gl::Texture* incidence() const;

    Scene& updateLightmap();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
