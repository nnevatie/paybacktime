#pragma once

#include "ext/mc_tables.h"
#include "mesh.h"
#include "sdf_primitives.h"

namespace hc
{
namespace McMesher
{

std::vector<glm::vec3> cellTriangles(const glm::vec3& origin, float r)
{
    std::vector<glm::vec3> triangles;
    return triangles;
}

template <typename T>
Mesh mesh(const T& sdf, float interval)
{
    Mesh mesh;
    return mesh;
}

} // namespace McMesher
} // namespace
