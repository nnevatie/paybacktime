#include "scene.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/random.hpp>

#include "common/log.h"
#include "img/color.h"

namespace pt
{

Box Scene::Item::bounds() const
{
    return {trRot.tr, obj.model().dimensions()};
}

Scene::Item::operator==(const Scene::Item& other) const
{
    return obj == other.obj && trRot == other.trRot;
}

Scene::Item::operator!=(const Scene::Item& other) const
{
    return !operator==(other);
}

struct Scene::Data
{
    Data()
    {}

    std::vector<Item> items;
    Image             lightmap;
    gl::Texture       lightTex;
};

Scene::Scene() :
    d(std::make_shared<Data>())
{
    updateLightmap();
}

Box Scene::bounds() const
{
    Box box;
    for (const auto& item : d->items)
        box |= item.bounds();
    return box;
}

bool Scene::contains(const Scene::Item& item) const
{
    return containsItem(d->items, item);
}

Scene& Scene::add(const Item& item)
{
    d->items.emplace_back(item);
    updateLightmap();
    return *this;
}

bool Scene::remove(const Item& item)
{
    for (int i = 0; i < int(d->items.size()); ++i)
        if (d->items.at(i).trRot.tr == item.trRot.tr)
        {
            d->items.erase(d->items.begin() + i);
            updateLightmap();
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

gl::Texture* Scene::lightmap() const
{
    return &d->lightTex;
}

Scene& Scene::updateLightmap()
{
    auto box  = bounds();
    auto size = glm::ceil(box.size.xz() / 8.f);

    HCLOG(Info) << "size: " << box.size.x << ", "
                            << box.size.y << ", "
                            << box.size.z << " -> "
                            << size.x     << " x "
                            << size.y     << " px";

    d->lightmap = Image(Size<int>(size.x, size.y), 4);
    for (int y = 0; y < size.y; ++y)
        for (int x = 0; x < size.x; ++x)
        {
            uint32_t* p = reinterpret_cast<uint32_t*>(d->lightmap.bits(x, y));
            *p = argb(glm::linearRand(0, 255));
        }

    d->lightmap.write("c:/temp/lightmap.png");
    d->lightTex.bind().alloc(d->lightmap)
                      .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                      .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return *this;
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
