#include "context.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "platform/clock.h"

namespace pt
{
namespace platform
{

struct Context::Data
{
    Data()
    {
        HCTIME("Construct");

        if (SDL_Init(SDL_INIT_VIDEO))
            throw std::runtime_error("SDL init failed.");

        const int sdlImageFlags = IMG_INIT_PNG | IMG_INIT_JPG;
        if (IMG_Init(sdlImageFlags) != sdlImageFlags)
            throw std::runtime_error("SDL image init failed.");
    }

    virtual ~Data()
    {
        HCTIME("Clean up");
        IMG_Quit();
        SDL_Quit();
    }
};

Context::Context() :
    d(std::make_shared<Data>())
{
}

} // namespace platform
} // namespace pt
