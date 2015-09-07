#include "ref_mesher.h"

#include <sstream>

#include <glm/geometric.hpp>

#include "clock.h"

namespace hc
{

namespace RefMesher
{

Mesh mesh(const sdf::Sphere& sphere)
{
    HCTIME("Extract sphere, r: " + std::to_string(sphere.r));
    return {};
}

Mesh mesh(const sdf::Box& box)
{
    HCTIME("Extract box, b: " + std::to_string(glm::length<float>(box.b)));

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
