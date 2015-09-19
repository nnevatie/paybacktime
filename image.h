#pragma once

#include <memory>
#include <string>

#include <SDL2/SDL.h>

#include "geometry.h"

namespace hc
{

struct Image
{
    Image();
    Image(const Size<int>& size, int depth);
    Image(const Size<int>& size, int depth, int stride);
    Image(const std::string& filename, int depth = 0);

    operator bool() const;

    Size<int> size() const;

    int depth() const;
    int stride() const;

    uint8_t* bits();
    const uint8_t* bits() const;
    SDL_Surface* surface() const;

    bool write(const std::string& filename) const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace
