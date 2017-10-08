#pragma once

#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include <glm/gtx/component_wise.hpp>

#include "ray.h"
#include "common/log.h"

namespace pt
{

struct Aabb
{
    typedef glm::vec3 V;

    Aabb()
    {}

    Aabb(const V& pos, const V& size) :
        pos(pos), size(size)
    {}

    inline V min() const
    {
        return pos;
    }

    inline V max() const
    {
        return pos + size;
    }

    inline V center() const
    {
        return pos + 0.5f * size;
    }

    inline explicit operator bool() const
    {
        return pos != V() || size != V();
    }

    inline bool operator==(const Aabb& box) const
    {
        return pos == box.pos && size == box.size;
    }

    inline bool operator!=(const Aabb& box) const
    {
        return !operator==(box);
    }

    inline Aabb operator|(const Aabb& aabb) const
    {
        if (!*this) return aabb;
        if (!aabb)  return *this;
        auto posMin = glm::min(pos, aabb.pos);
        auto posMax = glm::max(pos + size, aabb.pos + aabb.size);
        return {posMin, posMax - posMin};
    }

    inline Aabb& operator|=(const Aabb& aabb)
    {
        *this = *this | aabb;
        return *this;
    }

    inline Aabb rotated(int r)
    {
        const auto c = center();
        const auto s = 0.5f * size;
        switch (r & 0x03)
        {
            case 0:
            case 2:
                return *this;
            case 1:
            case 3:
                return Aabb(c - V(s.z, s.y, s.x), V(size.z, size.y, size.x));
        }
        return {};
    }

    inline bool intersect(const Aabb& aabb) const
    {
        const V a0 = min();
        const V a1 = max();
        const V b0 = aabb.min();
        const V b1 = aabb.max();
        return !(b0.x >= a1.x || b1.x <= a0.x ||
                 b0.y >= a1.y || b1.y <= a0.y ||
                 b0.z >= a1.z || b1.z <= a0.z);
    }

    inline bool intersect(const Ray& ray) const
    {
        V t0         = (min() - ray.pos) * ray.dirInv;
        V t1         = (max() - ray.pos) * ray.dirInv;
        V tMin       = glm::min(t0, t1);
        V tMax       = glm::max(t0, t1);
        float cMin   = glm::compMax(tMin);
        float cMax   = glm::compMin(tMax);
        return cMax >= cMin;
    }

    V pos, size;
};

} // namespace
