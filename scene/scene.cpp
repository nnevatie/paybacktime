#include "scene.h"

#include <vector>

#include <glm/gtc/random.hpp>
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
using ArchivePosRot   = std::tuple<float, float, float, int>;
using ArchivePosRots  = std::vector<ArchivePosRot>;
using ArchiveObjItems = std::unordered_map<Object::Id, ArchivePosRots>;

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
                for (const auto& pr : objItem.second)
                    d->objectItems.push_back(
                        ObjectItem(obj, {glm::vec3(std::get<0>(pr),
                                                   std::get<1>(pr),
                                                   std::get<2>(pr)),
                                                   std::get<3>(pr)}));
            }
            else
                PTLOG(Warn) << "object not found: " << objItem.first;
    }
    updateLightmap();
}

Box Scene::bounds() const
{
    Box box;
    for (const auto& item : d->objectItems)
        box |= item.bounds();
    return box;
}

glm::ivec3 Scene::cellResolution() const
{
    return {glm::ceil(bounds().size.xzy() / c::cell::SIZE.xzy())};
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
    d->objectItems.emplace_back(item);
    updateLightmap();
    return *this;
}

bool Scene::remove(const ObjectItem& item)
{
    for (int i = 0; i < int(d->objectItems.size()); ++i)
        if (d->objectItems.at(i) == item)
        {
            d->objectItems.erase(d->objectItems.begin() + i);
            updateLightmap();
            return true;
        }

    return false;
}

Scene& Scene::add(const CharacterItem& item)
{
    d->charItems.emplace_back(item);
    return *this;
}

bool Scene::contains(const Box& bounds) const
{
    for (const auto& i : d->objectItems)
        if (i.bounds() == bounds)
            return true;

    return false;
}

ObjectItems Scene::intersect(const ObjectItem& item) const
{
    ObjectItems items;
    const auto bounds = item.bounds();
    for (const auto& i : d->objectItems)
        if (i.bounds().intersect(bounds))
            items.push_back(i);

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
            items.emplace_back(item);

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
        auto gt = item.obj.transparent() ? GeometryType::Transparent :
                                           GeometryType::Opaque;

        if (type == GeometryType::Any || gt == type)
        {
            auto m = static_cast<glm::mat4x4>(item.posRot);
            instances.push_back({item.obj.model().primitive(), m});
        }
    }
    return instances;
}

gfx::Geometry::Instances Scene::characterGeometry() const
{
    const float s = 25.f;
    gfx::Geometry::Instances instances;
    instances.reserve(Character::PART_COUNT * d->charItems.size());
    for (const auto& item : d->charItems)
    {
        auto m = static_cast<glm::mat4x4>(item.posRot);
        for (const auto& bone : *item.obj.bones())
        {
            if (const auto& obj = bone.first)
            {
                auto mj  = bone.second;
                mj[3]   *= glm::vec4(s, s, s, 1.f);
                auto mo  = glm::translate(obj.origin());
                instances.push_back({obj.model().primitive(), m * mj * mo});
            }
        }
    }
    return instances;
}

gl::Texture* Scene::lightmap() const
{
    return d->lightmapper.lightTexture();
}

gl::Texture* Scene::incidence() const
{
    return d->lightmapper.incidenceTexture();
}

Scene& Scene::updateLightmap()
{
    const auto box  = bounds();
    const auto size = cellResolution();

    d->lightmapper.reset(size);
    for (const auto& item : d->objectItems)
    {
        const auto& obj     = item.obj;
        const auto density  = obj.density();
        const auto emission = obj.emission();
        const auto pos      = glm::ivec3((item.posRot.pos - box.pos).xzy() /
                                          c::cell::SIZE.xzy());
        const auto rot      = Rotation(density.size, item.posRot.rot);

        d->lightmapper.add(pos, rot, density, emission);
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
            const auto& pr = objItem.posRot;
            objItems[objItem.obj.id()].push_back(
                std::make_tuple(pr.pos.x, pr.pos.y, pr.pos.z, pr.rot));
        }
        ar(cereal::make_nvp("object_items", objItems));
        PTLOG(Info) << "unique object items: " << objItems.size();
    }
    return true;
}

} // namespace pt
