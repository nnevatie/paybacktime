#pragma once

#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "display.h"
#include "painter.h"
#include "image.h"
#include "clock.h"
#include "log.h"
#include "sdf_primitives.h"

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

        const sdf::Sphere sphere(1.f);
        HCLOG(Info) << "Sphere d: " << sphere(glm::vec3(0, 0, 0));
        HCLOG(Info) << "Sphere d: " << sphere(glm::vec3(1, 0, 0));
        HCLOG(Info) << "Sphere d: " << sphere(glm::vec3(2, 0, 0));

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
