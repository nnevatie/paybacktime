#pragma once

#include "geometry.h"
#include "sdf_primitives.h"

namespace hc
{

namespace ReferenceExtractor
{

Geometry extract(const sdf::Sphere& sphere);
Geometry extract(const sdf::Box& box);

} // namespace ReferenceExtractor
} // namespace
