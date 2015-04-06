#pragma once

#include <memory>
#include <string>

#include <SDL2/SDL.h>

#include "rect.h"

namespace hc
{

struct Image
{
    Image();
    Image(const std::string& filename, int depth = 0);

    operator bool() const;

    Rect<int> rect() const;
    int depth() const;

    unsigned char* bits() const;
    SDL_Surface* surface() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace
