#pragma once

#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include <glm/gtx/component_wise.hpp>

#include "ray.h"

namespace pt
{

struct Box
{
    typedef glm::vec3 V;

    Box()
    {}

    Box(const V& pos, const V& size) :
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

    inline operator bool() const
    {
        return pos != V() || size != V();
    }

    inline Box operator|(const Box& box) const
    {
        if (!*this) return box;
        if (!box)   return *this;
        auto posMin = glm::min(pos, box.pos);
        auto posMax = glm::max(pos + size, box.pos + box.size);
        return {posMin, posMax - posMin};
    }

    inline Box& operator|=(const Box& box)
    {
        *this = *this | box;
        return *this;
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
