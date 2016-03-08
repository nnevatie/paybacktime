#include "scene.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>

#include "common/log.h"

namespace pt
{

struct Scene::Data
{
    Data()
    {}

    std::vector<Item> items;
};

Scene::Scene() :
    d(std::make_shared<Data>())
{
}

bool Scene::contains(const Scene::Item& item) const
{
    return containsItem(d->items, item);
}

Scene& Scene::add(const Item& item)
{
    d->items.emplace_back(item);
    return *this;
}

bool Scene::remove(const Item& item)
{
    for (int i = 0; i < int(d->items.size()); ++i)
        if (d->items.at(i).trRot.tr == item.trRot.tr)
        {
            d->items.erase(d->items.begin() + i);
            return true;
        }

    return false;
}

Scene::Intersection Scene::intersect(const Ray& ray) const
{
    float di = 0;
    glm::intersectRayPlane(ray.pos, ray.dir, glm::vec3(), glm::vec3(0, 1, 0), di);
    const auto pos = ray.pos + di * ray.dir;

    Items items;
    for (const auto& item : d->items)
        if (item.bounds().intersect(ray))
            items.emplace_back(item);

    return {pos, items};
}

gfx::Geometry::Instances Scene::geometryInstances() const
{
    gfx::Geometry::Instances instances;
    instances.reserve(d->items.size());
    for (const auto& item : d->items)
    {
        auto m = static_cast<glm::mat4x4>(item.trRot);
        instances.push_back({item.obj.model().primitive(), m});
    }
    //HCLOG(Info) << instances.size();
    return instances;
}

bool containsItem(const Scene::Items& items, const Scene::Item& item)
{
    for (const auto& i : items)
        if (i.obj == item.obj && i.trRot.tr == item.trRot.tr)
            return true;

    return false;
}

bool containsObject(const Scene::Items& items, const Object& object)
{
    for (const auto& item : items)
        if (item.obj == object)
            return true;

    return false;
}

} // namespace pt
