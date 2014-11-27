#pragma once

#include "geometry.h"
#include "sdf_primitives.h"

namespace hc
{

namespace ReferenceExtractor
{

Geometry extract(const sdf::Sphere& sphere);

} // namespace SpecializedExtractor
} // namespace
