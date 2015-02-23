#pragma once

#include "geometry.h"
#include "sdf_primitives.h"

namespace hc
{

namespace McMesher
{

template <typename T>
Geometry geometry(const T& sdf)
{
    Geometry geometry;
    return geometry;
}

} // namespace McMesher
} // namespace
