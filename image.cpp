#include "image.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ext/stb_image_write.h"

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

Image::Image(int width, int height, int depth, int stride) :
    d(new Data {})
{
    d->width  = width;
    d->height = height;
    d->depth  = depth;
    d->stride = stride;
    d->bits   = new uint8_t[height * stride];
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

Size<int> Image::size() const
{
    return Size<int>(d->width, d->height);
}

int Image::depth() const
{
    return d->depth;
}

int Image::stride() const
{
    return d->stride;
}

uint8_t *Image::bits()
{
    return d->bits;
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

bool Image::write(const std::string& filename) const
{
    return !stbi_write_bmp(filename.c_str(),
                           d->width, d->height, d->depth, d->bits);
}

} // namespace
