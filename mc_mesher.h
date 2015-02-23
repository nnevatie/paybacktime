#pragma once

#include "mc_tables.h"
#include "geometry.h"
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
Geometry geometry(const T& sdf, float interval)
{
    Geometry geometry;
    return geometry;
}

} // namespace McMesher
} // namespace
