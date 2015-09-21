#include "painter.h"

#include <stdexcept>

#include "clock.h"

namespace hc
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

} // namespace

