#pragma once

#include <string>

#include <SDL2/SDL.h>

namespace hc
{

struct Display
{
    Display(const std::string& title, int width, int height);

    ~Display();

    SDL_Surface* surface() const;

    bool open();
    bool close();
    bool update();

private:

    std::string title_;
    int width_;
    int height_;

    SDL_Window* window_;
};

} // namespace
