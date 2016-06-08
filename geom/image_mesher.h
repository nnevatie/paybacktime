#pragma once

#include "img/image_cube.h"
#include "img/image.h"
#include "mesh.h"

namespace pt
{
namespace ImageMesher
{

Mesh_P_N_T_UV mesh(const Image& image,
                   float scale = 1.f);

Mesh_P_N_T_UV mesh(const ImageCube& imageCube,
                   const RectCube<float>& uvCube,
                   float scale = 1.f);

} // namespace ImageMesher
} // namespace pt
