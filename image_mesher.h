#pragma once

#include "mesh.h"
#include "image.h"
#include "image_cube.h"

namespace hc
{
namespace ImageMesher
{

Mesh mesh(const Image& image, float interval = 1.f);
Mesh mesh(const ImageCube& imageCube, float interval = 1.f);

} // namespace ImageMesher
} // namespace hc
