#pragma once

#include <glm/vec3.hpp>

namespace pt
{
namespace geom
{

struct Smooth
{
    Smooth() : iterations(0), strength(0.f)
    {}

    int   iterations;
    float strength;
};

struct Simplify
{
    Simplify() :  iterations(0), strength(0.f), scale(0.f)
    {}

    int   iterations;
    float strength;
    float scale;
};

struct Meta
{
    Meta() : scale(1.f)
    {}

    float    scale;
    Smooth   smooth;
    Simplify simplify;
};

} // namespace geom
} // namespace pt
