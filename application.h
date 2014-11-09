#pragma once

#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "display.h"
#include "painter.h"
#include "image.h"
#include "clock.h"
#include "log.h"

namespace hc
{

struct Application
{
    Application()
    {
        HCTIME("Construct");

        // Init SDL
        if (SDL_Init(SDL_INIT_VIDEO))
            throw std::runtime_error("SDL init failed.");

        // Init SDL image
        const int sdlImageFlags = IMG_INIT_PNG | IMG_INIT_JPG;
        if (IMG_Init(sdlImageFlags) != sdlImageFlags)
            throw std::runtime_error("SDL image init failed.");
    }

    ~Application()
    {
        HCTIME("Clean up");

        // Clean up SDL image
        IMG_Quit();

        // Clean up SDL
        SDL_Quit();
    }

    bool run()
    {
        hc::Display display("High Caliber", 800, 640);
        display.open();

        Image image("cat_life.jpg");

        Painter painter(display.surface());
        painter.drawImage(image, 0, 0);
        display.update();

        bool running = true;
        while (running)
        {
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_KEYDOWN)
                    running = false;
            }
        }
        return true;
    }
};

} // namespace
