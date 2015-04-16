#include "image_mesher.h"

#include "log.h"

namespace hc
{
namespace ImageMesher
{

namespace
{

struct Block
{
    BoundingBox bounds;
};

}

Geometry geometry(const Image& image, float interval)
{
    const Rect<int>             rect   = image.rect();
    const uint8_t* __restrict__ bits   = image.bits();
    const int                   stride = image.stride();

    for (float fy = rect.y; fy < rect.y + rect.h; fy += interval)
    {
        const int        y = int(fy + 0.5f);
        const uint8_t* row = bits + y * stride;

        for (float fx = rect.x; fx < rect.x + rect.w; fx += interval)
        {
            const int     x = int(fx + 0.5f);
            const uint8_t p = row[x];
            HCLOG(Info) << x << ", " << y << ": " << int(p);
        }
    }

    Geometry geometry;
    return geometry;
}

} // namespace ImageMesher
} // namespace

