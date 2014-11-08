#pragma once

#include <SDL2/SDL.h>
#include "image.h"

namespace hc
{

struct Painter
{
    Painter(SDL_Surface* surface) :
        surface_(surface)
    {
        if (!surface)
            throw std::invalid_argument("Cannot paint on a null-surface.");
    }

    bool drawImage(const Image &image, int x, int y)
    {
        HCTIME("time");
        SDL_Rect dstRect = {x, y, 0, 0};
        return !SDL_BlitSurface(image.surface(), nullptr, surface_, &dstRect);
    }

private:

    SDL_Surface* surface_;
};

} // namespace

