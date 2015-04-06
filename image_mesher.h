#pragma once

#include "geometry.h"
#include "image.h"

namespace hc
{
namespace ImageMesher
{

Geometry geometry(const Image& image, float interval = 1.f);

} // namespace ImageMesher
} // namespace

