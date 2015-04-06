#include "image_mesher.h"
#include "log.h"

namespace hc
{
namespace ImageMesher
{

Geometry geometry(const Image& image, float interval)
{
    const Rect<int> rect = image.rect();
    HCLOG(Info) << __FUNCTION__ << " " << image.depth() << " " << rect;

    Geometry geometry;
    return geometry;
}

} // namespace ImageMesher
} // namespace

