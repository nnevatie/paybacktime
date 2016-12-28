#include "context.h"

#include <SDL2/SDL.h>

#include "platform/clock.h"

namespace pt
{
namespace platform
{

struct Context::Data
{
    Data()
    {
        PTTIME("Construct");

        if (SDL_Init(SDL_INIT_VIDEO))
            throw std::runtime_error("SDL init failed.");
    }

    virtual ~Data()
    {
        PTTIME("Clean up");
        SDL_Quit();
    }
};

Context::Context() :
    d(std::make_shared<Data>())
{
}

} // namespace platform
} // namespace pt
