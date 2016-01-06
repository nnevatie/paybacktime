#include "image.h"

#include <algorithm>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "platform/clock.h"
#include "common/log.h"

namespace pt
{

struct Image::Data
{
    Size<int>    size;
    int          depth, stride;
    uint8_t*     bits;
    SDL_Surface* surface;

    Data() : depth(0), stride(0), bits(0), surface(0)
    {}

    Data(const Size<int>& size, int depth, int stride) :
        size(size), depth(depth), stride(stride),
        bits(new uint8_t[size.h * stride]),
        surface(0)
    {}

    virtual ~Data()
    {
        SDL_FreeSurface(surface);
        delete bits;
    }
};

Image::Image() :
    d(new Data())
{
}

Image::Image(const Size<int> &size, int depth) :
    Image(size, depth, size.w * depth)
{
}

Image::Image(const Size<int>& size, int depth, int stride) :
    d(new Data(size, depth, stride))
{
}

Image::Image(const std::string& filename, int depth) :
    d(new Data())
{
    d->bits   = stbi_load(filename.c_str(),
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
    if (!d->surface)
        d->surface = SDL_CreateRGBSurfaceFrom(
                         d->bits, d->size.w, d->size.h, d->depth * 8, d->stride,
                         0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    return d->surface;
}

Image Image::flipped() const
{
    Image image(clone());

    // Flip vertically
    const int h      = image.d->size.h;
    const int stride = image.d->stride;

    std::vector<uint8_t> t(static_cast<std::size_t>(stride));
    uint8_t* b = image.bits();

    for (int y = 0; y < h / 2; ++y)
    {
        uint8_t* r0 = b + y * stride;
        uint8_t* r1 = b + (h - y - 1) * stride;
        std::copy(r0, r0 + stride, t.data());
        std::copy(r1, r1 + stride, r0);
        std::copy(t.data(), t.data() + stride, r1);
    }
    return image;
}

Image Image::clone() const
{
    Image image(d->size, d->depth, d->stride);
    std::copy(d->bits, d->bits + (d->size.h * d->stride), image.d->bits);
    return image;
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
