#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "geom/grid.h"

namespace pt
{
namespace mat
{

using Density   = Grid<glm::vec4>;
using Light     = Grid<glm::vec3>;
using Indidence = Grid<glm::vec3>;
using Emission  = Grid<glm::vec3>;
using Pulse     = glm::vec2;

} // namespace mat
} // namespace pt
