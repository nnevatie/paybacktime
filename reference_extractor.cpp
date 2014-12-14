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
    return
    {
        // Vertices
        {},
        // Indices
        {}
    };
}

} // namespace SpecializedExtractor
} // namespace
