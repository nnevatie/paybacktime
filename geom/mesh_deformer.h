#pragma once

#include "mesh.h"

namespace pt
{
namespace MeshDeformer
{
// Types
using Mesh = Mesh_P_N_T_UV;

Mesh decimate(const Mesh& mesh0, int vertexCount);
Mesh smooth(const Mesh& mesh0, int iterCount);

} // namespace ImageDeformer
} // namespace pt
