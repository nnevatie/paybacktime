#include "image.h"

#include <algorithm>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/component_wise.hpp>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#include <nanovg.h>

#include "platform/clock.h"
#include "common/log.h"

#include "color.h"

namespace pt
{

struct Image::Data
{
    Size<int>    size;
    int          depth, stride;
    uint8_t*     bits;
    SDL_Surface* surface;
    NVGcontext*  nanoVg;
    int          nvgImage;

    Data() :
        depth(0), stride(0),
        bits(nullptr), surface(nullptr), nanoVg(nullptr), nvgImage(0)
    {}

    Data(const Size<int>& size, int depth, int stride) :
        size(size), depth(depth), stride(stride),
        bits(new uint8_t[size.h * stride]),
        surface(nullptr),
        nanoVg(nullptr),
        nvgImage(0)
    {}

    virtual ~Data()
    {
        SDL_FreeSurface(surface);
        if (nanoVg)
            nvgDeleteImage(nanoVg, nvgImage);
        delete bits;
    }
};

Image::Image() :
    d(std::make_shared<Data>())
{
}

Image::Image(const Size<int>& size, int depth) :
    Image(size, depth, size.w * depth)
{
}

Image::Image(const Size<int>& size, int depth, int stride) :
    d(std::make_shared<Data>(size, depth, stride))
{
}

Image::Image(const fs::path& path, int depth) :
    d(std::make_shared<Data>())
{
    d->bits = stbi_load(path.string().c_str(),
                        &d->size.w, &d->size.h, &d->depth, depth);
    if (depth > 0)
        d->depth = depth;

    d->stride = d->size.w * d->depth;
}

Image::operator bool() const
{
    return d && d->size && d->bits;
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

bool Image::channelPopulated(int index, uint8_t ref) const
{
    const uint32_t shift = index << 3;
    const uint32_t mask  = 0xff  << shift;
    for (int y = 0; y < d->size.h; ++y)
    {
        const uint32_t* __restrict__ row = bits<const uint32_t>(0, y);
        for (int x = 0; x < d->size.w; ++x)
            if (((row[x] & mask) >> shift) != ref)
                return true;
    }
    return false;
}

const uint8_t* Image::bits() const
{
    return d->bits;
}

uint8_t* Image::bits()
{
    return d->bits;
}

const uint8_t* Image::bits(int x, int y) const
{
    return d->bits + y * d->stride + d->depth * x;
}

uint8_t* Image::bits(int x, int y)
{
    return d->bits + y * d->stride + d->depth * x;
}

const uint8_t* Image::bitsClamped(int x, int y) const
{
    return bits(std::min(d->size.w - 1, std::max(0, x)),
                std::min(d->size.h - 1, std::max(0, y)));
}

SDL_Surface* Image::surface() const
{
    if (!d->surface)
    {
        d->surface = SDL_CreateRGBSurfaceFrom(
                         d->bits, d->size.w, d->size.h, d->depth * 8, d->stride,
                         0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

        SDL_SetSurfaceBlendMode(d->surface, SDL_BLENDMODE_NONE);
    }
    return d->surface;
}

int Image::nvgImage(NVGcontext* nanoVg) const
{
    if (!d->nanoVg)
    {
        d->nanoVg   = nanoVg;
        d->nvgImage = nvgCreateImageRGBA(
                          nanoVg, d->size.w, d->size.h, 0, d->bits);
    }
    return d->nvgImage;
}

Image Image::scaled(const Size<int>& size) const
{
    Image image(size, d->depth);
    stbir_resize_uint8(
        d->bits, d->size.w, d->size.h, d->stride,
        image.d->bits, image.d->size.w, image.d->size.h, image.d->stride,
        d->depth);

    return image;
}

Image Image::flipped(Axis axis) const
{
    if (!*this) return Image();

    Image image(d->size, d->depth);
    const int w      = image.d->size.w;
    const int h      = image.d->size.h;
    const int stride = image.d->stride;

    if (axis == Axis::X)
    {
        // Flip vertically
        for (int y = 0; y < h; ++y)
            std::copy(d->bits + y * stride,
                      d->bits + y * stride + stride,
                      image.d->bits + (h - y - 1) * stride);
    }
    else
    {
        // Flip horizontally
        if (d->depth == 1)
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x)
                    *image.bits<uint8_t>(x, y) =
                        *bits<const uint8_t>(w - x - 1, y);
        else
        if (d->depth == 4)
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x)
                    *image.bits<uint32_t>(x, y) =
                        *bits<const uint32_t>(w - x - 1, y);
        else
            throw std::runtime_error("Not implemented for depth: " +
                                     std::to_string(d->depth));
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

Image Image::normals(float strenght) const
{
    if (d->depth == 1)
    {
        Image image(d->size, 4);
        const int w = image.d->size.w;
        const int h = image.d->size.h;

        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                uint8_t tl = *bitsClamped(x - 1, y - 1);
                uint8_t  t = *bitsClamped(x + 0, y - 1);
                uint8_t tr = *bitsClamped(x + 1, y - 1);
                uint8_t  l = *bitsClamped(x - 1, y + 0);
                uint8_t  r = *bitsClamped(x + 1, y + 0);
                uint8_t bl = *bitsClamped(x - 1, y + 1);
                uint8_t  b = *bitsClamped(x + 0, y + 1);
                uint8_t br = *bitsClamped(x + 1, y + 1);

                float dx = (1.f / 255) * (tr + 2 * r + br - tl - 2 * l - bl);
                float dy = (1.f / 255) * (bl + 2 * b + br - tl - 2 * t - tr);

                glm::vec3  d(dx, dy, 1.f / strenght);
                glm::uvec3 n(255.f * (0.5f * (glm::normalize(d) + 1.f)) + 0.5f);
                *image.bits<uint32_t>(x, y) = argb(n);
            }

        return image;
    }
    return Image();
}

Image Image::maxToAlpha() const
{
    Image image(d->size, 4);
    if (d->depth == 4)
    {
        const int w = image.d->size.w;
        const int h = image.d->size.h;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                auto argb0 = *bits<const uint32_t>(x, y);
                auto  rgb0 = argbTuple(argb0).rgb();
                auto    a1 = glm::compMax(rgb0);
                auto argb1 = argb(glm::uvec4(rgb0, a1));
                *image.bits<uint32_t>(x, y) = argb1;
            }
    }
    else
        throw std::runtime_error("Not implemented for depth: " +
                                 std::to_string(d->depth));
    return image;
}

bool Image::write(const fs::path& path) const
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
