#pragma once

#include <glm/vec3.hpp>

#include "img/image_cube.h"
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
              int vertexCount);

Mesh smooth(const Mesh& mesh0,
            int iterCount);

} // namespace ImageDeformer
} // namespace pt
