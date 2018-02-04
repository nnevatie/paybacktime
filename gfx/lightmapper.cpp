#include "lightmapper.h"

#include <set>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>

#include "common/log.h"

#include "constants.h"

namespace pt
{
namespace gfx
{
namespace
{

struct ivec4_cmp : std::binary_function<glm::ivec4, glm::ivec4, bool>
{
    bool operator()(const glm::ivec4& lhs, const glm::ivec4 rhs) const
    {
        return glm::any(glm::lessThan(lhs, rhs));
    }
};
using Emitters = std::set<glm::ivec4, ivec4_cmp>;

void accumulate(
    mat::Density& density0,
    mat::Emission& emission0,
    Emitters& emitters,
    const Transform& xform,
    const mat::Density& density1,
    const mat::Emission& emission1,
    const mat::Pulse& pulse1)
{
    const auto pos     = xform.pos.xzy() +
                         glm::vec3(0.f, 0.f, 0.5f * glm::vec3(density1.size).z);
    const auto rot     = Transform::rotation(c::grid::UP, xform.rot);
    const auto aabb    = density1.bounds(pos).rotated(c::grid::UP, xform.rot);
    //const auto size0   = glm::ivec3(glm::ceil(aabb.size()));
    const auto size1   = density1.size;
    const auto min0    = glm::max(glm::zero<glm::ivec3>(),
                                  glm::ivec3(glm::floor(aabb.min)));
    const auto max0    = glm::min(density0.size,
                                  glm::ivec3(glm::ceil(aabb.max)));
    const auto origin0 = pos - glm::vec3(0.5f);
    const auto origin1 = 0.5f * glm::vec3(size1) - glm::vec3(0.5f);
    const auto pulse   = int(pulse1.x * 255 + 0.5f) |
                        (int(pulse1.y * 255 + 0.5f) << 8);
    #if 0
    PTLOG(Info) << "pos: " << glm::to_string(pos)
                << ", aabb: " << glm::to_string(aabb.min)
                << " -> " << glm::to_string(aabb.max)
                << ", min0: " << glm::to_string(min0)
                << ", max0: " << glm::to_string(max0)
                << ", size0: " << glm::to_string(size0)
                << ", size1: " << glm::to_string(size1)
                << ", origin0: " << glm::to_string(origin0)
                << ", origin1: " << glm::to_string(origin1);
    #endif
    glm::vec3 p0;
    for (p0.z = min0.z; p0.z < max0.z; ++p0.z)
        for (p0.y = min0.y; p0.y < max0.y; ++p0.y)
            for (p0.x = min0.x; p0.x < max0.x; ++p0.x)
            {
                const auto xv = glm::vec3(rot * glm::vec4(p0 - origin0, 1.f));
                const auto p1 = glm::ivec3(glm::round(origin1 + xv));

                if (glm::all(glm::greaterThanEqual(p1, glm::zero<glm::ivec3>())) &&
                    glm::all(glm::lessThan(p1, size1)))
                {
                    #if 0
                    PTLOG(Info) << "p0: "   << glm::to_string(p0)
                                << ", p1: " << glm::to_string(p1)
                                << ", " << glm::to_string(origin1 + xv);
                    #endif

                    auto& d0        = density0.at(p0);
                    const auto& d1  = density1.at(p1);

                    const auto aMin = std::min(d0.a, d1.a);
                    const auto a    = aMin < 0.f ? aMin : std::min(1.f, d0.a + d1.a);
                    const auto rgb  = d0.rgb() + d1.rgb();

                    d0 = glm::vec4(rgb, a);

                    const auto em1 = emission1.at(p1);
                    if (em1 != glm::zero<glm::vec3>())
                    {
                        auto em0 = emission0.at(p0);
                        emission0.at(p0) = em0 + em1;
                        emitters.insert({p0, pulse});
                    }
                }
            }

    #if 0
    for (int z = 0; z < density1.size.z; ++z)
        image(density1, z).write("c:/temp/density/src_" +
                                 std::to_string(z) + ".png");

    for (int z = 0; z < density0.size.z; ++z)
        image(density0, z).write("c:/temp/density/dest_" +
                                 std::to_string(z) + ".png");
    #endif
}

} // namespace

struct Lightmapper::Data
{
    Data() = default;

    mat::Density  density;
    mat::Emission emission;
    Emitters      emitters;
    Lightmap      lightmap;
};

Lightmapper::Lightmapper() :
    d(std::make_shared<Data>())
{}

gl::Texture* Lightmapper::lightTexture() const
{
    return &d->lightmap.light().second;
}

gl::Texture* Lightmapper::incidenceTexture() const
{
    return &d->lightmap.incidence().second;
}

Lightmap& Lightmapper::map() const
{
    return d->lightmap;
}

Lightmapper& Lightmapper::reset(const glm::ivec3& size)
{
    d->lightmap.resize(size);
    d->density  = mat::Density(size);
    d->emission = mat::Emission(size);
    d->emitters.clear();
    return *this;
}

Lightmapper& Lightmapper::add(const Transform& xform,
                              const mat::Density& density,
                              const mat::Emission& emission,
                              const mat::Pulse& pulse)
{
    accumulate(d->density, d->emission, d->emitters,
               xform, density, emission, pulse);
    return *this;
}

Lightmapper& Lightmapper::operator()(const Horizon& horizon)
{
    Lightmap::Emitters emitters;
    emitters.reserve(d->emitters.size());
    for (const auto& e : d->emitters)
        emitters.push_back({int16_t(e.x), int16_t(e.y),
                            int16_t(e.z), int16_t(e.w)});

    d->lightmap.update(d->density, d->emission, emitters, horizon);
    return *this;
}

} // namespace gfx
} // namespace pt
