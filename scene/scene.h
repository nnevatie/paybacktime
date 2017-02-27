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

#include "horizon.h"
#include "object.h"
#include "character.h"

namespace pt
{

template <typename T>
struct SceneItem
{
    T              obj;
    TransformTrRot trRot;

    SceneItem()
    {}

    SceneItem(const T& obj, const TransformTrRot& trRot) :
        obj(obj), trRot(trRot)
    {}

    Box bounds() const
    {
        return Box(trRot.tr, obj.dimensions()).rotated(trRot.rot);
    }

    operator bool() const
    {
        return obj;
    }
    bool operator==(const SceneItem& other) const
    {
        return obj == other.obj && trRot == other.trRot;
    }
    bool operator!=(const SceneItem& other) const
    {
        return !operator==(other);
    }
};

typedef SceneItem<Object>                 ObjectItem;
typedef SceneItem<Character>              CharacterItem;
typedef std::vector<ObjectItem>           ObjectItems;
typedef std::vector<CharacterItem>        CharacterItems;
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

bool containsItem(const ObjectItems& items, const ObjectItem& item);
bool containsObject(const ObjectItems& items, const Object& object);

} // namespace pt
