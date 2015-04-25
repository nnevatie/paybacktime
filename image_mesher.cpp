#include "image_mesher.h"

#include <iostream>
#include <glm/ext.hpp>

#include "clock.h"
#include "log.h"

namespace hc
{
namespace ImageMesher
{

namespace
{

struct Block
{
    Block()
    {}

    Block(const glm::vec3& c0, const glm::vec3& c1) : bounds(c0, c1)
    {}

    BoundingBox bounds;

    operator bool() const
    {
        return std::abs(bounds.first.z - bounds.second.z) > 0;
    }

    friend std::ostream& operator<<(std::ostream& out, const Block& block)
    {
        out << glm::to_string(block.bounds.first) << ", "
            << glm::to_string(block.bounds.second);

        return out;
    }
};

bool append(Geometry* geometry, const Block& block)
{
    if (geometry && block)
    {
        const glm::vec3& c0 = block.bounds.first;
        const glm::vec3& c1 = block.bounds.second;
        const int ib        = geometry->vertices.size();

        const glm::vec3 vertices[] =
        {
            // Front
            {c0.x, c0.y, c0.z},
            {c1.x, c0.y, c0.z},
            {c1.x, c1.y, c0.z},
            {c0.x, c1.y, c0.z},
            // Back
            {c0.x, c0.y, c1.z},
            {c1.x, c0.y, c1.z},
            {c1.x, c1.y, c1.z},
            {c0.x, c1.y, c1.z}

        };
        geometry->vertices.insert(geometry->vertices.end(),
                                  std::begin(vertices), std::end(vertices));

        typedef uint16_t index;
        const uint16_t indices[] =
        {
            // Front
            index(ib + 0), index(ib + 1), index(ib + 2),
            index(ib + 2), index(ib + 3), index(ib + 0),
            // Top
            index(ib + 3), index(ib + 2), index(ib + 6),
            index(ib + 6), index(ib + 7), index(ib + 3),
            // Back
            index(ib + 7), index(ib + 6), index(ib + 5),
            index(ib + 5), index(ib + 4), index(ib + 7),
            // Bottom
            index(ib + 4), index(ib + 5), index(ib + 1),
            index(ib + 1), index(ib + 0), index(ib + 4),
            // Left
            index(ib + 4), index(ib + 0), index(ib + 3),
            index(ib + 3), index(ib + 7), index(ib + 4),
            // Right
            index(ib + 1), index(ib + 5), index(ib + 6),
            index(ib + 6), index(ib + 2), index(ib + 1)
        };
        geometry->indices.insert(geometry->indices.end(),
                                 std::begin(indices), std::end(indices));
        return true;
    }
    return false;
}

}

Geometry geometry(const Image& image, float interval)
{
    HCTIME(__FUNCTION__);

    const Rect<int>             rect   = image.rect();
    const uint8_t* __restrict__ bits   = image.bits();
    const int                   stride = image.stride();
    const float                depth   = std::min(rect.w, rect.h) / 255.f;

    const int cx = rect.w / interval;
    const int cy = rect.h / interval;

    std::vector<Block> blocks;
    blocks.reserve(cx * cy);

    for (int sy = 0; sy < cy; ++sy)
    {
        const float                  fy = rect.y + sy * interval;
        const int                     y = int(fy);
        const uint8_t* __restrict__ row = bits + y * stride;

        for (int sx = 0; sx < cx; ++sx)
        {
            const int fx = int(rect.x + sx * interval);
            const int x  = int(fx);
            const int p  = row[x];

            const glm::vec3 c0(fx, fy, 0.f);
            const glm::vec3 c1(fx + interval, fy + interval, p * depth);

            const Block block(c0, c1);
            blocks.push_back(block);

            //HCLOG(Info) << x << ", " << y << ": " << int(p) << ", " << block;
        }
    }

    Geometry geometry;
    for (const Block& block : blocks)
        append(&geometry, block);

    HCLOG(Info) << geometry;
    return geometry;
}

} // namespace ImageMesher
} // namespace

