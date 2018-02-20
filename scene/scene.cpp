#include "scene.h"

#include <vector>

#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/transform.hpp>

#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/archives/binary.hpp>

#include "constants.h"

#include "platform/clock.h"
#include "gfx/lightmapper.h"
#include "common/log.h"

namespace pt
{
using ArchivePosition   = std::tuple<float, float, float>;
using ArchiveRotation   = std::tuple<int, int>;
using ArchiveTransform  = std::tuple<ArchivePosition, ArchiveRotation>;
using ArchiveTransforms = std::vector<ArchiveTransform>;
using ArchiveObjItems   = std::unordered_map<Object::Id, ArchiveTransforms>;

struct Scene::Data
{
    Data() :
        horizon(Horizon::none())
    {}

    Horizon          horizon;
    ObjectItems      objectItems;
    CharacterItems   charItems;

    gfx::Lightmapper lightmapper;
};

Scene::Scene() :
    d(std::make_shared<Data>())
{
    updateLightmap();
}

Scene::Scene(const fs::path& path,
             const HorizonStore& horizonStore,
             const ObjectStore& objectStore) :
    d(std::make_shared<Data>())
{
    PTTIME("read");
    std::ifstream is(path.generic_string());
    cereal::BinaryInputArchive ar(is);
    {
        // Horizon
        std::string name;
        ar(cereal::make_nvp("horizon", name));
        d->horizon = horizonStore.horizon(name);
    }
    {
        // Object items
        ArchiveObjItems objItems;
        ar(cereal::make_nvp("object_items", objItems));

        for (const auto& objItem : objItems)
            if (Object obj = objectStore.object(objItem.first))
            {
                PTLOG(Info) << " read obj " << objItem.first;
                for (const auto& xform : objItem.second)
                {
                    const auto& position = std::get<0>(xform);
                    const auto& rotation = std::get<1>(xform);
                    d->objectItems.emplace_back(
                        ObjectItem(obj, {glm::vec3(std::get<0>(position),
                                                   std::get<1>(position),
                                                   std::get<2>(position)),
                                                   std::get<0>(rotation)}));
                }
            }
            else
                PTLOG(Warn) << "object not found: " << objItem.first;
    }
    updateLightmap();
}

Aabb Scene::bounds() const
{
    Aabb aabb;
    for (const auto& item : d->objectItems)
        aabb |= item.bounds();
    return aabb;
}

glm::ivec3 Scene::cellResolution() const
{
    return {glm::ceil(bounds().size().xzy() / c::cell::SIZE.xzy())};
}

Horizon Scene::horizon() const
{
    return d->horizon;
}

Scene& Scene::setHorizon(const Horizon& horizon)
{
    d->horizon = horizon;
    updateLightmap();
    return *this;
}

Scene& Scene::add(const ObjectItem& item)
{
    for (const auto& obj : item.obj.hierarchy())
        //if (const auto model = obj.model())
            d->objectItems.emplace_back(obj, item.xform);

    updateLightmap();
    return *this;
}

bool Scene::remove(const ObjectItem& item)
{
    int removeCount  = d->objectItems.size();
    const auto items = item.hierarchy();

    for (const auto& child : items)
        d->objectItems.erase(
            std::remove_if(
                d->objectItems.begin(), d->objectItems.end(),
                [&](const ObjectItem& item) {return item == child;}),
                d->objectItems.end());

    removeCount -= d->objectItems.size();
    if (removeCount > 0)
        updateLightmap();

    return removeCount > 0;
}

Scene& Scene::add(const CharacterItem& item)
{
    d->charItems.emplace_back(item);
    return *this;
}

bool Scene::contains(const ObjectItem& item) const
{
    for (const auto& i : d->objectItems)
        if (i.obj == item.obj && i.bounds() == item.bounds())
            return true;

    return false;
}

ObjectItems Scene::intersect(const ObjectItem& item, float eps) const
{
    ObjectItems items;
    const auto bounds = item.bounds().extended(glm::vec3(-eps));
    for (const auto& item : d->objectItems)
        if (item.bounds().intersect(bounds))
            items.push_back(item);

    return items;
}

Intersection Scene::intersect(const Ray& ray) const
{
    float di = 0;
    glm::intersectRayPlane(ray.pos, ray.dir, glm::vec3(), glm::vec3(0, 1, 0), di);
    const auto pos = ray.pos + di * ray.dir;

    ObjectItems items;
    for (const auto& item : d->objectItems)
        if (item.bounds().intersect(ray))
        {
            // Resolve parent, if any
            if (const auto parent = item.obj.parent())
                items.emplace_back(parent, item.xform);
            else
                items.emplace_back(item);
        }

    // Sort intersections by distance to ray origin
    std::sort(items.begin(), items.end(),
        [ray](const ObjectItem& a, const ObjectItem& b)
        {
            return glm::distance(ray.pos, a.bounds().center()) <
                   glm::distance(ray.pos, b.bounds().center());
        });

    return {pos, items};
}

gfx::Geometry::Instances Scene::objectGeometry(GeometryType type) const
{
    gfx::Geometry::Instances instances;
    instances.reserve(d->objectItems.size());
    for (const auto& item : d->objectItems)
    {
        const auto& obj = item.obj;
        const auto gt   = obj.transparent() ? GeometryType::Transparent :
                                              GeometryType::Opaque;

        if (type == GeometryType::Any || gt == type)
        {
            auto xform = item.xform.matrix(obj.dimensions(), obj.origin());
            if (const auto model = obj.model())
                instances.emplace_back(model.primitive(), xform);
        }
    }
    return instances;
}

gfx::Geometry::Instances Scene::characterGeometry() const
{
    const float s = 28.125f;
    gfx::Geometry::Instances instances;
    instances.reserve(Character::PART_COUNT * d->charItems.size());
    for (const auto& item : d->charItems)
        for (const auto& bone : *item.obj.bones())
            if (const auto& obj = bone.first)
            {
                auto mw  = glm::translate(item.xform.pos);
                auto mj  = bone.second;
                mj[3]   *= glm::vec4(s, s, s, 1.f);
                auto mo  = glm::translate(obj.origin());
                instances.emplace_back(obj.model().primitive(), mw * mj * mo);
            }

    return instances;
}

gfx::Lightmap& Scene::lightmap() const
{
    return d->lightmapper.map();
}

Scene& Scene::updateLightmap()
{
    const auto aabb = bounds();
    d->lightmapper.reset(cellResolution());
    #if 0
    PTLOG(Info) << "bounds: " << glm::to_string(aabb.min) << "->"
                              << glm::to_string(aabb.max) << ", size: "
                              << glm::to_string(aabb.size()) << ", cells: "
                              << glm::to_string(cellResolution());
    #endif

    // Objects
    for (const auto& item : d->objectItems)
    {
        const auto& obj  = item.obj;
        const auto xform = Transform(item.xform.pos - aabb.min, item.xform.rot);
        d->lightmapper.add(xform, obj);
    }
    // Characters
    for (const auto& item : d->charItems)
        for (const auto& bone : *item.obj.bones())
            if (const auto& obj = bone.first)
            {
                const auto s = 28.125f;
                auto mw      = glm::translate(item.xform.pos);
                auto mj      = bone.second;
                mj[3]       *= glm::vec4(glm::vec3(s), 1.f);
                auto mo      = glm::translate(obj.origin());
                auto xform   = mw * mj /** mo*/;
                auto pos     = ((glm::vec3(xform[3]) - aabb.min)).xzy();
                auto rot     = glm::mat3(mj);
                d->lightmapper.add(pos, rot, obj);
            }

    d->lightmapper(d->horizon);
    return *this;
}

Scene& Scene::animate(TimePoint time, Duration step)
{
    for (auto& charItem : d->charItems)
        charItem.obj.animate(time, step);

    return *this;
}

bool Scene::write(const boost::filesystem::path& path) const
{
    PTTIME("write");
    std::ofstream os(path.generic_string(), std::ios::binary);
    cereal::BinaryOutputArchive ar(os);
    {
        // Horizon
        ar(cereal::make_nvp("horizon", d->horizon.name()));
    }
    {
        // Object items
        ArchiveObjItems objItems;
        for (const auto& objItem : d->objectItems)
        {
            const auto& xf = objItem.xform;
            const auto position = std::make_tuple(xf.pos.x, xf.pos.y, xf.pos.z);
            const auto rotation = std::make_tuple(xf.rot, c::scene::ROT_TICKS);
            objItems[objItem.obj.id()].emplace_back(
                std::make_tuple(position, rotation));
        }
        ar(cereal::make_nvp("object_items", objItems));
        PTLOG(Info) << "unique object items: " << objItems.size();
    }
    return true;
}

} // namespace pt
