#pragma once

#include "img/image_cube.h"
#include "img/image.h"
#include "geom/meta.h"
#include "mesh.h"

namespace pt
{
namespace ImageMesher
{

Mesh_P_N_T_UV mesh(const Image& image,
                   const geom::Meta& geom);

Mesh_P_N_T_UV mesh(const ImageCube& imageCube,
                   const RectCube<float>& uvCube,
                   const geom::Meta& geom);

} // namespace ImageMesher
} // namespace pt
