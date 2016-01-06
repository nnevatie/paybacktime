#include "painter.h"

#include <stdexcept>

#include "platform/clock.h"

namespace pt
{

Painter::Painter(SDL_Surface* surface) :
    surface_(surface)
{
    if (!surface)
        throw std::invalid_argument("Cannot paint on a null-surface.");

    renderer_ = SDL_CreateSoftwareRenderer(surface_);
}

Painter::Painter(Image* image) :
    Painter(image->surface())
{
}

Painter::~Painter()
{
    SDL_DestroyRenderer(renderer_);
}

Painter& Painter::setColor(uint32_t color)
{
    SDL_SetRenderDrawColor(renderer_,
                           (color & 0x00ff0000) >> 16,
                           (color & 0x0000ff00) >> 8,
                           (color & 0x000000ff) >> 0,
                           (color & 0xff000000) >> 24);
    return *this;
}

Painter& Painter::drawRect(const Rect<int>& rect)
{
    const SDL_Rect sdlRect = {rect.x, rect.y, rect.size.w, rect.size.h};
    SDL_RenderDrawRect(renderer_, &sdlRect);
    return *this;
}

Painter& Painter::drawImage(const Image& image, int x, int y)
{
    SDL_Rect dstRect = {x, y, 0, 0};
    SDL_BlitSurface(image.surface(), nullptr, surface_, &dstRect);
    return *this;
}

Painter& Painter::drawImageClamped(const Image& image, int x, int y, int margin)
{
    const Size<int> s = image.size();
    const int d       = margin;

    // Left, right, top, bottom
    // Top-left, top-right, bottom-left, bottom-right
    SDL_Rect rects[8][2] = {
        {{0,       0,       1,   s.h}, {x,           y + d,       d,   s.h}},
        {{s.w - 1, 0,       1,   s.h}, {x + d + s.w, y + d,       d,   s.h}},
        {{0,       0,       s.w, 1},   {x + d,       y,           s.w, d}},
        {{0,       s.h - 1, s.w, 1},   {x + d,       y + d + s.h, s.w, d}},
        {{0,       0,       1,   1},   {x,           y,           d,   d}},
        {{s.w - 1, 0,       1,   1},   {x + d + s.w, y,           d,   d}},
        {{0,       s.h - 1, 1,   1},   {x,           y + d + s.h, d,   d}},
        {{s.w - 1, s.h - 1, 1,   1},   {x + d + s.w, y + d + s.h, d,   d}},
    };

    drawImage(image, x + margin, y + margin);
    for (int i = 0; i < 8; ++i)
        SDL_BlitScaled(image.surface(), &rects[i][0], surface_, &rects[i][1]);

    return *this;
}

} // namespace

