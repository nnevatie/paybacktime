#include "scene.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/random.hpp>
#include <glm/gtx/transform.hpp>

#include "platform/clock.h"
#include "common/log.h"
#include "img/color.h"

namespace pt
{

namespace
{

float vis(const Grid<float>& map, glm::ivec3 p0, glm::ivec3 p1)
{
    // TODO: find out whether useful
    if (p0.y > p1.y) std::swap(p0, p1);

    const auto d = p1 - p0;
    const auto n = glm::abs(d.x) > glm::abs(d.y) ?
                   glm::abs(d.x) : glm::abs(d.y);
    const auto s = glm::vec3(d) / float(n);

    float v = 1.f;
    auto p  = glm::vec3(p0) + s + 0.5f;

    for (int i = 0; i < n - 1 && v > 0; ++i, p += s)
        v -= map.at(int(p.x), int(p.y), int(p.z));

    return glm::max(0.f, v);
}

void accumulateVisibility(
    Grid<float>* map, const glm::ivec3& pos, const Grid<float>& visibility)
{
    auto const size = visibility.size;
    for (int z = 0; z < size.z; ++z)
    {
        const auto z1 = pos.z + z;
        for (int y = 0; y < size.x; ++y)
        {
            const auto y1 = pos.y + y;
            for (int x = 0; x < size.y; ++x)
            {
                const auto x1 = pos.x + x;
                map->at(x1, y1, z1) = glm::min(1.f, map->at(x1, y1, z1) +
                                                    visibility.at(x, y, z));
            }
        }
    }
}

void accumulateEmission(
    Grid<glm::vec3>* map, const glm::ivec3& pos, const Grid<glm::vec3>& emission)
{
    auto const size = emission.size;
    for (int z = 0; z < size.z; ++z)
    {
        const int z1 = pos.z + z;
        for (int y = 0; y < size.y; ++y)
        {
            const int y1 = pos.y + y;
            for (int x = 0; x < size.x; ++x)
            {
                const int x1        = pos.x + x;
                auto argb0          = map->at(x1, y1, z1);
                auto argb1          = emission.at(x, y);
                auto argb2          = argb0 + argb1;
                map->at(x1, y1, z1) = argb2;
            }
        }
    }
}

void accumulateLightmap(Grid<glm::vec3>* map,
                        const Grid<float>& visibility,
                        const Grid<glm::vec3>& emissive)
{
    auto const exp     = 1.f;
    auto const ambient = 0.f;
    auto const attMin  = 0.005f;
    auto const k0      = 1.f;
    auto const k1      = 0.0f;
    auto const k2      = 0.02f;
    auto const st      = glm::vec3(8.f, 8.f, 32.f);

    auto const size = map->size;
    for (int z = 0; z < size.z; ++z)
        for (int y = 0; y < size.y; ++y)
            for (int x = 0; x < size.x; ++x)
            {
                glm::vec3 sum;
                for (int ez = 0; ez < size.z; ++ez)
                    for (int ey = 0; ey < size.y; ++ey)
                        for (int ex = 0; ex < size.x; ++ex)
                        {
                            auto argb = emissive.at(ex, ey, ez);
                            if (argb != glm::zero<glm::vec3>())
                            {
                                auto e   = argb;
                                auto w0  = glm::vec3(x  * st.x, y  * st.y, z  * st.z);
                                auto w1  = glm::vec3(ex * st.x, ey * st.y, ez * st.z);
                                auto d   = glm::distance(w0, w1);
                                auto att = 1.f / (k0 + k1 * d + k2 * d * d);
                                if (att > attMin)
                                {
                                    auto v = vis(visibility, glm::ivec3(ex, ey, ez),
                                                             glm::ivec3(x,  y,  z));
                                    sum   += v * exp * e * att;
                                }
                            }
                        }

                map->at(x, y, z) = ambient + sum;
            }
}

}

Box Scene::Item::bounds() const
{
    return {trRot.tr, obj.model().dimensions() / obj.scale()};
}

bool Scene::Item::operator==(const Scene::Item& other) const
{
    return obj == other.obj && trRot == other.trRot;
}

bool Scene::Item::operator!=(const Scene::Item& other) const
{
    return !operator==(other);
}

struct Scene::Data
{
    Data() : lightTex(gl::Texture::Type::Texture3d)
    {}

    std::vector<Item> items;

    Grid<float>       visibility;
    Grid<glm::vec3>   emissive;
    Grid<glm::vec3>   lightmap;

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
    HCTIME("generate lighting");

    auto box = bounds();
    glm::ivec3 size(glm::ceil(box.size.xz() / 8.f),
                    glm::ceil(box.size.y    / 32.f));

    // Visibility and emissive
    d->visibility = Grid<float>(size);
    d->emissive   = Grid<glm::vec3>(size);

    for (const auto& item : d->items)
    {
        const auto visibility = item.obj.visibility();
        const auto emission   = item.obj.emission();
        const auto pos        = glm::ivec3(((item.trRot.tr - box.pos) / 8.f).xz(),
                                           ((item.trRot.tr - box.pos) / 32.f).y);

        accumulateVisibility(&d->visibility, pos, visibility);
        accumulateEmission(&d->emissive, pos, emission);
    }

    // Lightmap
    d->lightmap = Grid<glm::vec3>(size);
    accumulateLightmap(&d->lightmap, d->visibility, d->emissive);

    image(d->visibility).write("c:/temp/visibility.png");
    image(d->emissive).write("c:/temp/emissive.png");
    image(d->lightmap).write("c:/temp/lightmap.png");

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
