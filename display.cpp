#include "display.h"

#include "clock.h"
#include "log.h"

namespace hc
{

Display::Display(const std::string& title, int width, int height) :
    title_(title),
    width_(width),
    height_(height),
    window_(nullptr),
    glContext_(nullptr)
{
}

Display::~Display()
{
    close();
}

SDL_Surface* Display::surface() const
{
    return SDL_GetWindowSurface(window_);
}

bool Display::open()
{
    HCTIME("");
    if (!window_)
    {
        // Create window
        window_ = SDL_CreateWindow(title_.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width_,
            height_,
            SDL_WINDOW_OPENGL);

        // Create OpenGL context
        // TODO: Define requested version/profile
        glContext_ = SDL_GL_CreateContext(window_);

        // TODO: Check return values
        return true;
    }
    else
        HCLOG(Warn) << "Window already open.";

    return false;
}

bool Display::close()
{
    if (window_)
    {
        SDL_GL_DeleteContext(glContext_);
        SDL_DestroyWindow(window_);
        return true;
    }
    else
        HCLOG(Warn) << "Window not open.";

    return false;
}

bool Display::update()
{
    return !SDL_UpdateWindowSurface(window_);
}

bool Display::swap()
{
    if (window_)
    {
        SDL_GL_SwapWindow(window_);
        return true;
    }
    return false;
}

} // namespace
