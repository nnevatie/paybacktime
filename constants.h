#pragma once

#include <glm/vec3.hpp>

namespace pt
{
namespace c
{

namespace scene
{
    // World vectors
    const auto UP    = glm::vec3(0.f, 1.f, 0.f),
               FWD   = glm::vec3(0.f, 0.f, 1.f),
               RIGHT = glm::vec3(1.f, 0.f, 0.f);

    // Rotation ticks
    const auto ROT_TICKS = 16;
}

namespace cell
{
    // X/Z form the ground plane, Y points World-up
    const auto SIZE = glm::vec3(8.f,  8.f, 8.f),
               GRID = glm::vec3(16.f, 1.f, 16.f);
}

namespace grid
{
    // Grid-up
    const auto UP = glm::vec3(0.f, 0.f, 1.f);
}

namespace object
{
    constexpr auto METAFILE = "object.json";
    constexpr auto EXPOSURE = 0.25f;

    namespace meta
    {
        constexpr auto BASE     = "base",
                       ORIGIN   = "origin",
                       SCALE    = "scale",
                       PULSE    = "pulse",
                       SMOOTH   = "smooth",
                       SIMPLIFY = "simplify",
                       CHILDREN = "children";
    }
}

namespace character
{
    constexpr auto METAFILE = "character.json";

    namespace skeleton
    {
        constexpr auto SCALE = 28.125f;
    }
}

} // namespace c
} // namespace pt
