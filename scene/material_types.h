#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "geom/grid.h"

namespace pt
{
namespace mat
{

using Density   = Grid<float>;
using Light     = Grid<glm::vec3>;
using Indidence = Grid<glm::vec3>;
using Emission  = Grid<glm::vec3>;
using Bleed     = Grid<glm::vec4>;

} // namespace mat
} // namespace pt
