#pragma once

#include <glm/vec3.hpp>

namespace pt
{

struct Ray
{
    typedef glm::vec3 V;

    Ray()
    {}

    Ray(const V& pos, const V& dir) :
        pos(pos), dir(dir), dirInv(1.f / dir)
    {}

    V pos, dir, dirInv;
};

} // namespace
