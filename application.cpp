#include "application.h"

#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>

#include "display.h"
#include "painter.h"
#include "clock.h"
#include "log.h"

#include "sdf_primitives.h"
#include "reference_extractor.h"

#include "gl_buffers.h"

namespace hc
{

Application::Application()
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

Application::~Application()
{
    HCTIME("Clean up");

    // Clean up SDL image
    IMG_Quit();

    // Clean up SDL
    SDL_Quit();
}

bool Application::run()
{
    hc::Display display("High Caliber", 800, 640);
    display.open();

    /*
    Image image("cat_life.jpg");
    Painter painter(display.surface());
    painter.drawImage(image, 0, 0);
    display.update();
    */

    const sdf::Sphere sphere(1.f);
    HCLOG(Info) << "Sphere d: " << sphere({0, 0, 0});
    HCLOG(Info) << "Sphere d: " << sphere({1, 0, 0});
    HCLOG(Info) << "Sphere d: " << sphere({2, 0, 0});

    const sdf::Box box({2, 1, 1});
    HCLOG(Info) << "Box d: " << box({0, 0, 0});
    HCLOG(Info) << "Box d: " << box({1, 0, 0});
    HCLOG(Info) << "Box d: " << box({2, 0, 0});
    HCLOG(Info) << "Box d: " << box({3, 0, 0});

    const Geometry boxGeometry = ReferenceExtractor::extract(box);

    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    display.swap();

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

} // namespace
