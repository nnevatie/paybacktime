#include "image.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "clock.h"
#include "log.h"

namespace hc
{

struct Image::Data
{
    int width,
        height,
        depth,
        stride;

    unsigned char* bits;
};

Image::Image() :
    d(new Data {})
{
}

Image::Image(const std::string& filename) :
    d(new Data {})
{
    HCTIME(__FUNCTION__);
    d->bits = stbi_load(filename.c_str(), &d->width, &d->height, &d->depth, 0);
    d->stride = d->width * d->depth;
}

SDL_Surface* Image::surface() const
{
    return SDL_CreateRGBSurfaceFrom(
        d->bits, d->width, d->height, d->depth * 8, d->stride,
        0x000000ff, 0x00ff0000, 0x0000ff00, 0xff000000);
}

} // namespace
