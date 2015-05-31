#include "image.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

    uint8_t* bits;

    Data() : width(), height(), depth(), stride(), bits()
    {
    }

    virtual ~Data()
    {
        delete bits;
    }
};

Image::Image() :
    d(new Data {})
{
}

Image::Image(const std::string& filename, int depth) :
    d(new Data {})
{
    d->bits = stbi_load(filename.c_str(),
                        &d->width, &d->height, &d->depth, depth);
    d->stride = d->width * d->depth;
}

Image::operator bool() const
{
    return d && d->width && d->height;
}

Rect<int> Image::rect() const
{
    return Rect<int>(0, 0, d->width, d->height);
}

int Image::depth() const
{
    return d->depth;
}

int Image::stride() const
{
    return d->stride;
}

const uint8_t* Image::bits() const
{
    return d->bits;
}

SDL_Surface* Image::surface() const
{
    return SDL_CreateRGBSurfaceFrom(
        d->bits, d->width, d->height, d->depth * 8, d->stride,
        0x000000ff, 0x00ff0000, 0x0000ff00, 0xff000000);
}

} // namespace
