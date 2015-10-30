#pragma once

#include "mesh.h"
#include "image.h"
#include "image_cube.h"

namespace hc
{
namespace ImageMesher
{

Mesh_P_N_UV mesh(const Image& image,
            float interval = 1.f);

Mesh_P_N_UV mesh(const ImageCube& imageCube,
            const RectCube<float>& uvCube,
            float interval = 1.f);

} // namespace ImageMesher
} // namespace hc
