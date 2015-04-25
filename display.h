#pragma once

#include <string>

#include <SDL2/SDL.h>

namespace hc
{

struct Display
{
    Display(const std::string& title, int width, int height);
    ~Display();

    int width() const;
    int height() const;

    SDL_Surface* surface() const;

    bool open();
    bool close();
    bool update();
    bool swap();

private:

    std::string title_;
    int width_;
    int height_;

    SDL_Window* window_;
    SDL_GLContext glContext_;
};

} // namespace
