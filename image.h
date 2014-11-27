#pragma once

#include <string>

#include <SDL2/SDL_image.h>

namespace hc
{

struct Image
{
    Image();
    Image(const Image& /*image*/);
    Image(const std::string& filename);
    ~Image();

    SDL_Surface* surface() const;

private:

    SDL_Surface* surface_;
};

} // namespace
