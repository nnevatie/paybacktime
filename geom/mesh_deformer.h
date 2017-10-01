#pragma once

#include <glm/vec3.hpp>

#include "img/image_cube.h"
#include "geom/meta.h"
#include "mesh.h"

namespace pt
{
namespace MeshDeformer
{
// Types
using Mesh = Mesh_P_N_T_UV;

Mesh decimate(const Mesh& mesh0,
              const RectCube<float>& uvCube,
              const glm::vec3& size,
              const geom::Simplify& params);

Mesh smooth(const Mesh& mesh0,
            const geom::Smooth& params);

} // namespace ImageDeformer
} // namespace pt
