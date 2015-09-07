#pragma once

#include "mesh.h"
#include "sdf_primitives.h"

namespace hc
{
namespace RefMesher
{

Mesh mesh(const sdf::Sphere& sphere);
Mesh mesh(const sdf::Box& box);

} // namespace RefMesher
} // namespace
