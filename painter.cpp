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
}

bool Painter::drawImage(const Image& image, int x, int y)
{
    //HCTIME("time");
    SDL_Rect dstRect = {x, y, 0, 0};
    return !SDL_BlitSurface(image.surface(), nullptr, surface_, &dstRect);
}

} // namespace

