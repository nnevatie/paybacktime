#pragma once

#include <glm/vec3.hpp>

namespace pt
{
namespace geom
{

struct Smooth
{
    Smooth() : iterations(0), strength(0.5f)
    {}

    operator bool() const
    {
        return iterations > 0;
    }

    int   iterations;
    float strength;
};

struct Simplify
{
    Simplify() :  iterations(5), strength(0.01f), scale(2.f)
    {}

    operator bool() const
    {
        return iterations > 0;
    }

    int   iterations;
    float strength;
    float scale;
};

struct Meta
{
    Meta() = default;

    float    scale = 1.f;
    Smooth   smooth;
    Simplify simplify;
};

} // namespace geom
} // namespace pt
