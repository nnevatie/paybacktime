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

float vis(const Grid<float>& map, glm::ivec2 p0, glm::ivec2 p1)
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
        v -= map.at(int(p.x), int(p.y));
    }
    return glm::max(0.f, v);
}

void accumulateVisibility(
    Grid<float>* map, const glm::ivec2& pos, const Grid<float>& visibility)
{
    auto const size = visibility.size;
    for (int y = 0; y < size.h; ++y)
    {
        float* __restrict__ rowOut       = map->ptr(pos.y + y);
        const float* __restrict__ rowVis = visibility.ptr(y);
        for (int x = 0; x < size.w; ++x)
        {
            const int x1 = pos.x + x;
            rowOut[x1]   = glm::min(1.f, rowOut[x1] + rowVis[x]);
        }
    }
}

void accumulateEmission(
    Grid<glm::vec3>* map, const glm::ivec2& pos, const Grid<glm::vec3>& emission)
{
    auto const size = emission.size;
    for (int y = 0; y < size.h; ++y)
    {
        glm::vec3* __restrict__ rowOut        = map->ptr(pos.y + y);
        const glm::vec3* __restrict__ rowEmis = emission.ptr(y);

        for (int x = 0; x < size.w; ++x)
        {
            const int x1 = pos.x + x;
            auto argb0   = rowOut[x1];
            auto argb1   = rowEmis[x];
            auto argb2   = argb0 + argb1;
            rowOut[x1]   = argb2;
        }
    }
}

void accumulateLightmap(Grid<glm::vec3>* map,
                        const Grid<float>& visibility,
                        const Grid<glm::vec3>& emissive)
{
    auto const exp     = 1.f;
    auto const ambient = 0.f;
    auto const k0      = 1.f;
    auto const k1      = 0.05f;
    auto const k2      = 0.01f;
    auto const st      = glm::vec3(8.f, 8.f, 32.f);

    auto const size = map->size;
    for (int y = 0; y < size.h; ++y)
    {
        glm::vec3* __restrict__ rowOut = map->ptr(y);
        for (int x = 0; x < size.w; ++x)
        {
            glm::vec3 sum;
            for (int ey = 0; ey < size.h; ++ey)
            {
                const glm::vec3* __restrict__ rowEmis = emissive.ptr(ey);
                for (int ex = 0; ex < size.w; ++ex)
                {
                    auto argb = rowEmis[ex];
                    if (argb != glm::zero<glm::vec3>())
                    {
                        auto e  = argb;
                        auto w0 = glm::vec2(x  * st.x, y  * st.y);
                        auto w1 = glm::vec2(ex * st.x, ey * st.y);
                        auto d  = glm::max(0.f, glm::distance(w0, w1));
                        auto v  = vis(visibility, glm::ivec2(ex, ey),
                                                  glm::ivec2(x,  y));

                        auto att = 1.f / (k0 + k1 * d + k2 * d * d);
                        sum     += v * exp * e * att;
                    }
                }
            }
            rowOut[x] = ambient + sum;
        }
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

    auto box  = bounds();
    auto size = Size<int>(glm::ceil(box.size.xz() / 8.f));

    // Visibility and emissive
    d->visibility = Grid<float>(size);
    d->emissive   = Grid<glm::vec3>(size);

    for (const auto& item : d->items)
    {
        const auto visibility = item.obj.visibility();
        const auto emission   = item.obj.emission();
        const auto pos        = glm::ivec2(((item.trRot.tr - box.pos) / 8.f).xz());

        accumulateVisibility(&d->visibility, pos, visibility);
        accumulateEmission(&d->emissive, pos, emission);
    }

    // Lightmap
    d->lightmap = Grid<glm::vec3>(size);
    accumulateLightmap(&d->lightmap, d->visibility, d->emissive);

    image(d->visibility).write("c:/temp/visibility.png");
    image(d->emissive).write("c:/temp/emissive.png");
    image(d->lightmap).write("c:/temp/lightmap.png");

    d->lightTex.bind().alloc3d(d->lightmap)
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
