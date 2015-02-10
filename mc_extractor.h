#pragma once

#include "geometry.h"
#include "sdf_primitives.h"

namespace hc
{

namespace McExtractor
{

template <typename T>
Geometry extract(const T& sdf)
{
    Geometry geometry;
    return geometry;
}

} // namespace McExtractor
} // namespace
