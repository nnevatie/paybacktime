#pragma once

#include <glm/vec3.hpp>

namespace pt
{
namespace c
{

namespace cell
{
    // X/Z form the ground plane, Y points World-up
    const auto SIZE = glm::vec3(8.f, 16.f, 8.f);
    const auto GRID = glm::vec3(16.f, 0.f, 16.f);
}

namespace object
{
    constexpr auto SCALE    = 1.f,
                   EXPOSURE = 0.25f;
}

} // namespace c
} // namespace pt
