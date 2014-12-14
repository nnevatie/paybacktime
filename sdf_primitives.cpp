#include "sdf_primitives.h"

#include <glm/geometric.hpp>

namespace hc
{
namespace sdf
{

Sphere::Sphere(float r) : r(r) {}

float Sphere::operator()(const glm::vec3& p) const
{
    return glm::length(p) - r;
}

Box::Box(const glm::vec3& b) : b(b) {}

float Box::operator()(const glm::vec3& p) const
{
    const glm::vec3 d = glm::abs(p) - b;
    return glm::min(glm::max(d.x, glm::max(d.y, d.z)), 0.f) +
        glm::length(glm::max(d, 0.f));
}

} // namespace sdf
} // namespace hc
