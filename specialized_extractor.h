#pragma once

#include <sstream>

#include "geometry.h"
#include "sdf_primitives.h"
#include "clock.h"

namespace hc
{

namespace ReferenceExtractor
{

Geometry extract(const sdf::Sphere& sphere)
{
    HCTIME(str(std::stringstream() << "Extract sphere, r: " << sphere.r));
    return {};
}

} // namespace SpecializedExtractor

} // namespace
