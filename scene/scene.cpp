#include "scene.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/random.hpp>

#include "platform/clock.h"
#include "common/log.h"
#include "img/color.h"

namespace pt
{

namespace
{

float vis(const Image& map, glm::ivec2 p0, glm::ivec2 p1)
{
    // TODO: find out whether useful
    if (p0.y > p1.y) std::swap(p0, p1);

    const glm::ivec2 d = p1 - p0;
    const int n        = glm::abs(d.x) > glm::abs(d.y) ?
                         glm::abs(d.x) : glm::abs(d.y);
    const glm::vec2 s  = {float(d.x) / n, float(d.y) / n};

    float v     = 1.f;
    glm::vec2 p = glm::vec2(p0) + 0.5f;

    for (int i = 0; i < n - 1 && v > 0; ++i)
    {
        p += s;
        if (uint8_t b = *map.bits(p.x, p.y))
            v -= b / 255.f;
    }
    return glm::max(0.f, v);
}

void accumulateVisibility(
    Image* map, const glm::ivec2& pos, const Image& visibility)
{
    auto const size = visibility.size();
    for (int y = 0; y < size.h; ++y)
    {
        uint8_t* __restrict__ rowOut =
            reinterpret_cast<uint8_t*>(map->bits(0, pos.y + y));

        const uint8_t* __restrict__ rowVis =
            reinterpret_cast<const uint8_t*>(visibility.bits(0, y));

        for (int x = 0; x < size.w; ++x)
        {
            const int x1 = pos.x + x;
            rowOut[x1]   = glm::min(255, rowOut[x1] + rowVis[x]);
        }
    }
}

void accumulateEmission(
    Image* map, const glm::ivec2& pos, const Image& emission)
{
    auto const size = emission.size();
    for (int y = 0; y < size.h; ++y)
    {
        uint32_t* __restrict__ rowOut =
            reinterpret_cast<uint32_t*>(map->bits(0, pos.y + y));

        const uint32_t* __restrict__ rowEmis =
            reinterpret_cast<const uint32_t*>(emission.bits(0, y));

        for (int x = 0; x < size.w; ++x)
        {
            const int x1 = pos.x + x;
            auto argb0   = argbTuple(rowOut[x1]);
            auto argb1   = argbTuple(rowEmis[x]);
            auto argb2   = argb0 + argb1;
            rowOut[x1]   = argb(glm::min(glm::uvec4(255), argb2));
        }
    }
}

void accumulateLightmap(
    Image* map, const Image& visibility, const Image& emissive)
{
    auto const exp     = 1.f;
    auto const falloff = 1.f;
    auto const ambient = 1.f;

    auto const size = map->size();
    for (int y = 0; y < size.h; ++y)
    {
        uint32_t* __restrict__ rowOut =
            reinterpret_cast<uint32_t*>(map->bits(0, y));

        for (int x = 0; x < size.w; ++x)
        {
            glm::vec3 sum;
            for (int ey = 0; ey < size.h; ++ey)
            {
                const uint32_t* __restrict__ rowEmis =
                    reinterpret_cast<const uint32_t*>(emissive.bits(0, ey));

                for (int ex = 0; ex < size.w; ++ex)
                {
                    auto argb = rowEmis[ex];
                    if (argb != 0)
                    {
                        auto e = glm::vec3(argbTuple(argb).rgb());
                        auto d = glm::max(1.f, glm::distance(glm::vec2(x,   y),
                                                             glm::vec2(ex, ey)));
                        auto v = vis(visibility, glm::ivec2(ex, ey), glm::ivec2(x, y));
                        sum   += (v * exp * e) / glm::pow(d, falloff);
                    }
                }
            }
            rowOut[x] = argb(glm::min(glm::vec4(255.f),
                                      glm::vec4(ambient + sum, 255.f)));
        }
    }
}

}

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

    Image             visibility;
    Image             emissive;
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
    HCTIME("generate lighting");

    auto box  = bounds();
    auto size = Size<int>(glm::ceil(box.size.xz() / 8.f));

    // Visibility and emissive
    d->visibility = Image(size, 1);
    d->emissive   = Image(size, 4);
    d->visibility.fill(0x00000000);
    d->emissive.fill(0x00000000);

    for (const auto& item : d->items)
    {
        const auto visibility = item.obj.visibility();
        const auto emission   = item.obj.emission();
        const auto pos        = glm::ivec2(((item.trRot.tr - box.pos) / 8.f).xz());

        accumulateVisibility(&d->visibility, pos, visibility);
        accumulateEmission(&d->emissive, pos, emission);
    }

    // Lightmap
    d->lightmap = Image(size, 4);
    accumulateLightmap(&d->lightmap, d->visibility, d->emissive);

    d->visibility.write("c:/temp/visibility.png");
    d->emissive.write("c:/temp/emissive.png");
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
