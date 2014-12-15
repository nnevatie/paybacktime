#include "reference_extractor.h"

#include <sstream>

#include <glm/geometric.hpp>

#include "clock.h"

namespace hc
{

namespace ReferenceExtractor
{

Geometry extract(const sdf::Sphere& sphere)
{
    HCTIME(str(std::stringstream() << "Extract sphere, r: " << sphere.r));
    return {};
}

Geometry extract(const sdf::Box& box)
{
    HCTIME(str(std::stringstream() << "Extract box, b: " << glm::length(box.b)));
    const glm::vec3& v = box.b;
    return
    {
        // Vertices
        {
            // Front
            {-v.x, -v.y,  v.z},
            { v.x, -v.y,  v.z},
            { v.x,  v.y,  v.z},
            {-v.x,  v.y,  v.z},
            // Back
            {-v.x, -v.y, -v.z},
            { v.x, -v.y, -v.z},
            { v.x,  v.y, -v.z},
            {-v.x,  v.y, -v.z}
        },
        // Indices
        {
            // Front
            0, 1, 2,
            2, 3, 0,
            // Top
            3, 2, 6,
            6, 7, 3,
            // Back
            7, 6, 5,
            5, 4, 7,
            // Bottom
            4, 5, 1,
            1, 0, 4,
            // Left
            4, 0, 3,
            3, 7, 4,
            // Right
            1, 5, 6,
            6, 2, 1
        }
    };
}

} // namespace SpecializedExtractor
} // namespace
