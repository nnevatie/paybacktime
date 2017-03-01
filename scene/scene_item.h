#pragma once

#include <vector>
#include <utility>

#include "geom/box.h"
#include "geom/transform.h"

#include "object.h"
#include "character.h"
#include "horizon.h"

namespace pt
{

template <typename T>
struct SceneItem
{
    T           obj;
    PosRotation posRot;

    SceneItem()
    {}

    SceneItem(const T& obj, const PosRotation& posRot) :
        obj(obj), posRot(posRot)
    {}

    Box bounds() const
    {
        return Box(posRot.pos, obj.dimensions()).rotated(posRot.rot);
    }
    operator bool() const
    {
        return obj;
    }
    bool operator==(const SceneItem& other) const
    {
        return obj == other.obj && posRot == other.posRot;
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
