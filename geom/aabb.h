#pragma once

#include <array>

#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/string_cast.hpp>

#include "common/log.h"

#include "transform.h"
#include "ray.h"

namespace pt
{

struct Aabb
{
    using V = glm::vec3;
    using Vertices = std::array<V, 8>;

    Aabb()
    {}

    Aabb(const V& max) :
        min(glm::zero<V>()), max(max)
    {}

    Aabb(const V& min, const V& max) :
        min(min), max(max)
    {}

    Aabb(const std::array<V, 8>& vertices) :
        min(std::numeric_limits<float>::max()),
        max(std::numeric_limits<float>::lowest())
    {
        for (const auto& v : vertices)
        {
            min = glm::min(min, v);
            max = glm::max(max, v);
        }
        #if 0
        PTLOG(Info) << "min: " << glm::to_string(min) << ", "
                    << "max: " << glm::to_string(max);
        #endif
    }

    inline V size() const
    {
        return max - min;
    }

    inline V center() const
    {
        return 0.5f * (min + max);
    }

    inline explicit operator bool() const
    {
        return min != V() || max != V();
    }

    inline bool operator==(const Aabb& aabb) const
    {
        return min == aabb.min && max == aabb.max;
    }

    inline bool operator!=(const Aabb& aabb) const
    {
        return !operator==(aabb);
    }

    inline Aabb operator|(const Aabb& aabb) const
    {
        if (!*this) return aabb;
        if (!aabb)  return *this;
        return {glm::min(min, aabb.min), glm::max(max, aabb.max)};
    }

    inline Aabb& operator|=(const Aabb& aabb)
    {
        *this = *this | aabb;
        return *this;
    }

    inline Aabb extended(const glm::vec3& v) const
    {
        return {min - v, max + v};
    }

    inline Aabb rotated(const glm::vec3& axis, int r, const glm::vec3& origin) const
    {
        return rotated(Transform::rotation(axis, r), origin);
    }

    inline Aabb rotated(const glm::mat4x4& rot, const glm::vec3& origin) const
    {
        const auto o = center() + origin;
        const auto x = glm::translate(o) * rot * glm::translate(-o);
        auto verts   = vertices();
        for (auto& v : verts)
            v = x * glm::vec4(v, 1.f);

        return Aabb(verts);
    }

    inline bool intersect(const Aabb& aabb) const
    {
        const V a0 = min;
        const V a1 = max;
        const V b0 = aabb.min;
        const V b1 = aabb.max;
        return !(b0.x >= a1.x || b1.x <= a0.x ||
                 b0.y >= a1.y || b1.y <= a0.y ||
                 b0.z >= a1.z || b1.z <= a0.z);
    }

    inline bool intersect(const Ray& ray) const
    {
        V t0         = (min - ray.pos) * ray.dirInv;
        V t1         = (max - ray.pos) * ray.dirInv;
        V tMin       = glm::min(t0, t1);
        V tMax       = glm::max(t0, t1);
        float cMin   = glm::compMax(tMin);
        float cMax   = glm::compMin(tMax);
        return cMax >= cMin;
    }

    inline Vertices vertices() const
    {
        return
        {{
            V(min.x, min.y, min.z),
            V(min.x, min.y, max.z),
            V(max.x, min.y, max.z),
            V(max.x, min.y, min.z),

            V(max.x, max.y, max.z),
            V(max.x, max.y, min.z),
            V(min.x, max.y, min.z),
            V(min.x, max.y, max.z)
        }};
    }

    V min, max;
};

} // namespace
