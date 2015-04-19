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

    friend std::ostream& operator<<(std::ostream& out, const Block& block)
    {
        out << glm::to_string(block.bounds.first) << ", "
            << glm::to_string(block.bounds.second);

        return out;
    }
};

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

            HCLOG(Info) << x << ", " << y << ": " << int(p) << ", " << block;
        }
    }

    Geometry geometry;
    return geometry;
}

} // namespace ImageMesher
} // namespace

