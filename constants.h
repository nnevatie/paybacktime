#pragma once

#include <glm/vec3.hpp>

namespace pt
{
namespace c
{

namespace cell
{
// X/Z form the ground plane, Y points World-up
const auto SIZE = glm::vec3(8.f, 32.f, 8.f);
}

namespace object
{
constexpr auto SCALE    = 1.f,
               EXPOSURE = 0.15f;
}

} // namespace c
} // namespace pt
