#pragma once

#include "sdf_primitives.h"
#include "clock.h"

namespace hc
{

namespace SpecializedExtractor
{

void extract(const sdf::Sphere& /*sphere*/)
{
    HCTIME("Extract sphere");
}

} // namespace SpecializedExtractor

} // namespace
