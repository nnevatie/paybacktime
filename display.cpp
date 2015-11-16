#include <GL/glew.h>

#include "display.h"

#include "clock.h"
#include "log.h"

namespace hc
{

static void debugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const GLvoid* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB ||
        severity == GL_DEBUG_SEVERITY_HIGH_ARB)
        HCLOG(Debug) << message;
}

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

        // Set GL context attributes
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
        //                    SDL_GL_CONTEXT_DEBUG_FLAG);

        // Set GL buffer attributes
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        // Create window
        window_ = SDL_CreateWindow(title_.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            size_.w,
            size_.h,
            SDL_WINDOW_OPENGL |
            SDL_WINDOW_ALLOW_HIGHDPI);

        // Create OpenGL context
        // TODO: Define requested version/profile
        //       and check return value
        glContext_ = SDL_GL_CreateContext(window_);

        // Init GLEW
        const GLenum glewStatus = glewInit();

        if (glDebugMessageCallbackARB != nullptr)
        {
            glDebugMessageCallbackARB(debugCallback, 0);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            HCLOG(Info) << "GL debug callback supported";
        }

        // Set swap interval
        SDL_GL_SetSwapInterval(0);

        /*
        int dedicatedVideoMem = 0,
            totalAvailableMem = 0,
            curAvailableMem   = 0;

        glGetIntegerv(0x9047, &dedicatedVideoMem);
        glGetIntegerv(0x9048, &totalAvailableMem);
        glGetIntegerv(0x9049, &curAvailableMem);

        HCLOG(Debug) << dedicatedVideoMem << ", "
                     << totalAvailableMem << ", "
                     << curAvailableMem;
        */

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
    const int stride     = size.w * 4;

    Image image(size, 4, stride);
    glReadPixels(0, 0, size.w, size.h, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

    // Flip vertically
    // TODO: Use flip from Image
    std::vector<uint8_t> t(static_cast<std::size_t>(stride));
    uint8_t* b = image.bits();
    for (int y = 0; y < size.h / 2; ++y)
    {
        uint8_t* r0 = b + y * stride;
        uint8_t* r1 = b + (size.h - y - 1) * stride;
        std::copy(r0, r0 + stride, t.data());
        std::copy(r1, r1 + stride, r0);
        std::copy(t.data(), t.data() + stride, r1);
    }
    return image;
}

} // namespace
