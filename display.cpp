#include <GL/glew.h>

#include "display.h"

#include "clock.h"
#include "log.h"

namespace hc
{

Display::Display(const std::string& title, const Size<int>& size) :
    title_(title),
    size_(size),
    window_(nullptr),
    glContext_(nullptr)
{
}

Display::~Display()
{
    close();
}

Size<int> Display::size() const
{
    return size_;
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
        // Log available video drivers
        HCLOG(Debug) << "Video drivers:";
        const int driverCount = SDL_GetNumVideoDrivers();
        for (int i = 0; i < driverCount; ++i)
            HCLOG(Debug) << "#" << (i + 1) << ": "
                         << SDL_GetVideoDriver(i);

        // Create window
        window_ = SDL_CreateWindow(title_.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            size_.w,
            size_.h,
            SDL_WINDOW_OPENGL);

        // Create OpenGL context
        // TODO: Define requested version/profile
        //       and check return value
        glContext_ = SDL_GL_CreateContext(window_);

        // Init GLEW
        const GLenum glewStatus = glewInit();

        // Log renderer info
        HCLOG(Debug) << "OpenGL vendor: '" << glGetString(GL_VENDOR)
                     << "', renderer: '"   << glGetString(GL_RENDERER)
                     << "', version: '"    << glGetString(GL_VERSION) << "'";

        return glewStatus == GLEW_OK;
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

Image Display::capture() const
{
    const Size<int> size = this->size();
    const int stride = size.w * 4;

    Image image(size, 4, stride);
    glReadPixels(0, 0, size.w, size.h, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

    // Flip vertically
    uint8_t  t[stride];
    uint8_t* b = image.bits();
    for (int y = 0; y < size.h / 2; ++y)
    {
        uint8_t* r0 = b + y * stride;
        uint8_t* r1 = b + (size.h - y - 1) * stride;
        std::copy(r0, r0 + stride, t);
        std::copy(r1, r1 + stride, r0);
        std::copy(t,  t  + stride, r1);
    }

    return image;
}

} // namespace
