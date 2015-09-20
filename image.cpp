#include "image.h"

#include <algorithm>

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
    Size<int> size;
    int depth {}, stride {};
    uint8_t* bits {};

    virtual ~Data()
    {
        delete bits;
    }
};

Image::Image() :
    d(new Data {})
{
}

Image::Image(const Size<int> &size, int depth) :
    Image(size, depth, size.w * depth)
{
}

Image::Image(const Size<int>& size, int depth, int stride) :
    d(new Data {})
{
    d->size   = size;
    d->depth  = depth;
    d->stride = stride;
    d->bits   = new uint8_t[d->size.h * d->stride];
}

Image::Image(const std::string& filename, int depth) :
    d(new Data {})
{
    d->bits  = stbi_load(filename.c_str(),
                         &d->size.w, &d->size.h, &d->depth, depth);
    d->stride = d->size.w * d->depth;
}

Image::operator bool() const
{
    return d && d->size;
}

Size<int> Image::size() const
{
    return d->size;
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
        d->bits, d->size.w, d->size.h, d->depth * 8, d->stride,
                0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
}

Image& Image::fill(uint32_t value)
{
    uint32_t* p0 = reinterpret_cast<uint32_t*>(d->bits);
    uint32_t* p1 = reinterpret_cast<uint32_t*>(d->bits + d->size.h * d->stride);
    std::fill(p0, p1, value);
    return *this;
}

bool Image::write(const filesystem::path& path) const
{
    const std::string ext(path.extension().string());
    if (ext == ".png")
        return !stbi_write_png(path.string().c_str(),
                               d->size.w, d->size.h, d->depth,
                               d->bits, d->stride);
    else
    if (ext == ".bmp")
        return !stbi_write_bmp(path.string().c_str(),
                               d->size.w, d->size.h, d->depth,
                               d->bits);
    return false;
}

} // namespace
