#pragma once

#include "geometry.h"
#include "sdf_primitives.h"

namespace hc
{

namespace RefMesher
{

Geometry geometry(const sdf::Sphere& sphere);
Geometry geometry(const sdf::Box& box);

} // namespace RefMesher
} // namespace
