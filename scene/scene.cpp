#include "scene.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/random.hpp>
#include <glm/gtx/transform.hpp>

#include "constants.h"

#include "platform/clock.h"
#include "gfx/lightmapper.h"
#include "common/log.h"

namespace pt
{

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

Scene& Scene::setHorizon(const Horizon& horizon)
{
    d->horizon = horizon;
    updateLightmap();
    return *this;
}

bool Scene::contains(const ObjectItem& item) const
{
    return containsItem(d->objectItems, item);
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
        if (d->objectItems.at(i).trRot.tr == item.trRot.tr)
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

Intersection Scene::intersect(const Ray& ray) const
{
    float di = 0;
    glm::intersectRayPlane(ray.pos, ray.dir, glm::vec3(), glm::vec3(0, 1, 0), di);
    const auto pos = ray.pos + di * ray.dir;

    ObjectItems items;
    for (const auto& item : d->objectItems)
        if (item.bounds().intersect(ray))
            items.emplace_back(item);

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
            auto m = static_cast<glm::mat4x4>(item.trRot);
            instances.push_back({item.obj.model().primitive(), m});
        }
    }
    return instances;
}

gfx::Geometry::Instances Scene::characterGeometry() const
{
    gfx::Geometry::Instances instances;
    instances.reserve(d->charItems.size() * 15);
    for (const auto& item : d->charItems)
    {
        auto m = static_cast<glm::mat4x4>(item.trRot);
        for (const auto& obj : *item.obj.parts())
            if (obj) instances.push_back({obj.model().primitive(),
                                          m * glm::translate(obj.origin())});
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
        const auto pos = glm::ivec3((item.trRot.tr - box.pos).xzy() /
                                     c::cell::SIZE.xzy());

        d->lightmapper.add(pos, item.obj.density(), item.obj.emission());
    }
    d->lightmapper(d->horizon);
    return *this;
}

bool containsItem(const ObjectItems& items, const ObjectItem& item)
{
    for (const auto& i : items)
        if (i.obj == item.obj && i.trRot.tr == item.trRot.tr)
            return true;

    return false;
}

bool containsObject(const ObjectItems& items, const Object& object)
{
    for (const auto& item : items)
        if (item.obj == object)
            return true;

    return false;
}

} // namespace pt
