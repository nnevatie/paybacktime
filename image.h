#pragma once

#include <memory>
#include <string>

#include <SDL2/SDL.h>

namespace hc
{

struct Image
{
    Image();
    Image(const std::string& filename);

    SDL_Surface* surface() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace
