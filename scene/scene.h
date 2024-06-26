#pragma once

#include <memory>
#include <vector>
#include <utility>

#include <glm/vec3.hpp>

#include <cereal/access.hpp>

#include "geom/aabb.h"
#include "geom/ray.h"
#include "geom/transform.h"

#include "platform/clock.h"
#include "img/image.h"
#include "gfx/geometry.h"
#include "gfx/lightmap.h"
#include "gl/texture.h"

#include "horizon_store.h"
#include "object_store.h"
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
    Scene(const fs::path& path,
          const HorizonStore& horizonStore,
          const ObjectStore& objectStore);

    Aabb bounds() const;
    glm::ivec3 cellResolution() const;

    Horizon horizon() const;
    Scene& setHorizon(const Horizon& horizon);

    Scene& add(const ObjectItem& item);
    bool remove(const ObjectItem& item);

    Scene& add(const CharacterItem& item);

    bool contains(const ObjectItem& item) const;

    ObjectItems intersect(const ObjectItem& item, float eps = 0.f) const;
    Intersection intersect(const Ray& ray) const;

    gfx::Geometry::Instances objectGeometry(
        GeometryType type = GeometryType::Any) const;

    gfx::Geometry::Instances characterGeometry() const;

    gfx::Lightmap& lightmap() const;

    Scene& updateLightmap();

    Scene& animate(TimePoint time, Duration step);

    bool write(const fs::path& path) const;

private:
    struct Data;
    std::shared_ptr<Data> d;

    friend class cereal::access;

    template <class Archive>
    void serialize(Archive& ar);
};

} // namespace pt
