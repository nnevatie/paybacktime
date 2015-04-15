#include "image_mesher.h"

#include "log.h"

namespace hc
{
namespace ImageMesher
{

Geometry geometry(const Image& image, float interval)
{
    const Rect<int>             rect   = image.rect();
    const uint8_t* __restrict__ bits   = image.bits();
    const int                   stride = image.stride();

    for (int y = rect.y; y < rect.y + rect.h; ++y)
    {
        const uint8_t* row = (const uint8_t*) (bits + y * stride);
        for (int x = rect.x; x < rect.x + rect.w; ++x)
        {
            const uint8_t p = row[x];
            HCLOG(Info) << x << ", " << y << ": " << int(p);
        }
    }

    Geometry geometry;
    return geometry;
}

} // namespace ImageMesher
} // namespace

