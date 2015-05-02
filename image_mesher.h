#pragma once

#include "geometry.h"
#include "image.h"
#include "image_cube.h"

namespace hc
{
namespace ImageMesher
{

Geometry geometry(const Image& image, float interval = 1.f);
Geometry geometry(const ImageCube& imageCube, float interval = 1.f);

} // namespace ImageMesher
} // namespace hc

