#pragma once

#include <string>

#include <SDL2/SDL.h>

#include "geometry.h"
#include "image.h"

namespace hc
{
namespace ui
{

struct Display
{
    Display(const std::string& title, const Size<int>& size);
    ~Display();

    Size<int> size() const;

    SDL_Surface* surface() const;

    bool open();
    bool close();
    bool update();
    bool swap();

    Image capture() const;

private:

    std::string title_;
    Size<int> size_;

    SDL_Window* window_;
    SDL_GLContext glContext_;
};

} // namespace ui
} // namespace hc
