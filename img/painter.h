#pragma once

#include <SDL2/SDL.h>

#include "geom/rect.h"
#include "img/image.h"

namespace hc
{

struct Painter
{
    Painter(SDL_Surface* surface);
    Painter(Image* image);

    virtual ~Painter();

    Painter& setColor(uint32_t color);

    Painter& drawRect(const Rect<int>& rect);
    Painter& drawImage(const Image& image, int x, int y);
    Painter& drawImageClamped(const Image& image, int x, int y, int margin);

private:

    SDL_Surface* surface_;
    SDL_Renderer* renderer_;
};

} // namespace

