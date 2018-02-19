#pragma once

#include <vector>
#include <utility>

#include "geom/aabb.h"
#include "geom/transform.h"

#include "object.h"
#include "character.h"
#include "horizon.h"

namespace pt
{
template <typename T> struct SceneItem;
template <typename T> using SceneItems = std::vector<SceneItem<T>>;

template <typename T>
struct SceneItem
{
    T         obj;
    Transform xform;

    SceneItem() = default;

    SceneItem(const T& obj, const Transform& xform) :
        obj(obj), xform(xform)
    {}

    SceneItems<T> hierarchy() const
    {
        SceneItems<T> items;
        const auto children = obj.hierarchy();
        items.reserve(children.size());
        for (const auto& child : children)
            items.emplace_back(child, child.parent() ?
                                      xform + child.origin() : xform);
        return items;
    }

    Aabb bounds() const
    {
        const auto dim  = obj.dimensions();
        const auto hdim = glm::vec3(0.5f * dim.x, 0.f, 0.5f * dim.z);
        const auto min  = xform.pos - obj.origin() - hdim;
        const auto max  = min + dim;
        return Aabb(min, max).
               rotated(c::scene::UP, xform.rot, obj.origin());
    }

    operator bool() const
    {
        return obj;
    }
    bool operator==(const SceneItem& other) const
    {
        return obj == other.obj && xform == other.xform;
    }
    bool operator!=(const SceneItem& other) const
    {
        return !operator==(other);
    }
};

using ObjectItem     = SceneItem<Object>;
using CharacterItem  = SceneItem<Character>;
using ObjectItems    = std::vector<ObjectItem>;
using CharacterItems = std::vector<CharacterItem>;
using Intersection   = std::pair<glm::vec3, ObjectItems>;

} // namespace pt
