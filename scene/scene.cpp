#include "scene.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/random.hpp>
#include <glm/gtx/transform.hpp>

#include "constants.h"

#include "platform/clock.h"
#include "common/log.h"
#include "img/color.h"

namespace pt
{

namespace
{

float vis(const Grid<float>& map, glm::ivec3 p0, glm::ivec3 p1)
{
    // Make results symmetrical between endpoints
    if (p0.y > p1.y) std::swap(p0, p1);

    const auto d = p1 - p0;
    const auto n = glm::abs(d.x) > glm::abs(d.y) ?
                   glm::abs(d.x) : glm::abs(d.y);
    const auto s = glm::vec3(d) / float(n);

    float v = 1.f;
    auto p  = glm::vec3(p0) + s + 0.5f;

    for (int i = 0; i < n - 1 && v > 0.f; ++i, p += s)
        v -= map.at(int(p.x), int(p.y), int(p.z));

    return glm::max(0.f, v);
}

void accumulateDensity(
    Grid<float>* map, const glm::ivec3& pos, const Grid<float>& density)
{
    auto const size = density.size;
    for (int z = 0; z < size.z; ++z)
    {
        const auto z1 = pos.z + z;
        for (int y = 0; y < size.y; ++y)
        {
            const auto y1 = pos.y + y;
            for (int x = 0; x < size.x; ++x)
            {
                const auto x1 = pos.x + x;
                map->at(x1, y1, z1) = glm::min(1.f, map->at(x1, y1, z1) +
                                                    density.at(x, y, z));
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

void accumulateLightmap(Grid<glm::vec3>* lightmap,
                        Grid<glm::vec3>* incidence,
                        const Grid<float>& density,
                        const Grid<glm::vec3>& emissive)
{
    auto const exp     = 1.f;
    auto const ambient = 0.f;
    auto const attMin  = 0.005f;
    auto const k0      = 1.f;
    auto const k1      = 0.5f;
    auto const k2      = 0.05f;
    auto const st      = c::cell::SIZE.xzy();

    auto const size = lightmap->size;
    //#pragma omp parallel for
    for (int z = 0; z < size.z; ++z)
        for (int y = 0; y < size.y; ++y)
            for (int x = 0; x < size.x; ++x)
            {
                glm::vec3 light;
                glm::vec3 incid;
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
                                    auto v = vis(density,
                                                 glm::ivec3(ex, ey, ez),
                                                 glm::ivec3(x,  y,  z));
                                    light += v * v * exp * e * att;
                                    incid += att * (w1 - w0);
                                }
                            }
                        }

                lightmap->at(x, y, z)  = ambient + light;
                incidence->at(x, y, z) = incid;
            }
}

}

struct Scene::Data
{
    Data() : lightTex(gl::Texture::Type::Texture3d),
             incidenceTex(gl::Texture::Type::Texture3d)
    {}

    ObjectItems     objectItems;
    CharacterItems  charItems;

    Grid<float>     density;
    Grid<glm::vec3> emissive;
    Grid<glm::vec3> lightmap;
    Grid<glm::vec3> incidence;

    gl::Texture     lightTex;
    gl::Texture     incidenceTex;
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
    return &d->lightTex;
}

gl::Texture* Scene::incidence() const
{
    return &d->incidenceTex;
}

Scene& Scene::updateLightmap()
{
    const auto box  = bounds();
    const auto size = cellResolution();

    HCTIME("generate lighting " + std::to_string(size.x) + "x"
                                + std::to_string(size.y) + "x"
                                + std::to_string(size.z));
    // Density and emissive
    d->density  = Grid<float>(size);
    d->emissive = Grid<glm::vec3>(size);
    {
        HCTIME("precalc dens+emis maps");
        for (const auto& item : d->objectItems)
        {
            const auto density  = item.obj.density();
            const auto emission = item.obj.emission();
            const auto pos      = glm::ivec3((item.trRot.tr - box.pos).xzy() /
                                              c::cell::SIZE.xzy());

            accumulateDensity(&d->density, pos, density);
            accumulateEmission(&d->emissive, pos, emission);
        }
    }
    // Lightmap
    {
        HCTIME("acc lightmap");
        d->lightmap  = Grid<glm::vec3>(size);
        d->incidence = Grid<glm::vec3>(size);
        accumulateLightmap(&d->lightmap, &d->incidence,
                           d->density, d->emissive);
    }

    image(d->density).write("c:/temp/density.png");
    image(d->emissive).write("c:/temp/emissive.png");
    image(d->lightmap).write("c:/temp/lightmap.png");
    image(d->incidence).write("c:/temp/incidence.png");

    d->lightTex.bind().alloc(d->lightmap)
                      .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                      .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    d->incidenceTex.bind().alloc(d->incidence)
                          .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                          .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
