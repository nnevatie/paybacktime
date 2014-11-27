#pragma once

#include <SDL2/SDL.h>

#include "image.h"

namespace hc
{

struct Painter
{
    Painter(SDL_Surface* surface);

    bool drawImage(const Image &image, int x, int y);

private:

    SDL_Surface* surface_;
};

} // namespace

