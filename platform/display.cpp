#include "display.h"

#include <glad/glad.h>

#include "platform/clock.h"
#include "common/log.h"

namespace
{

void debugCallback(
    GLenum /*source*/, GLenum /*type*/, GLuint /*id*/,
    GLenum severity, GLsizei /*length*/,
    const GLchar* message, const GLvoid* /*userParam*/)
{
    if ((severity == GL_DEBUG_SEVERITY_MEDIUM_ARB ||
         severity == GL_DEBUG_SEVERITY_HIGH_ARB)  &&
        !strstr(message, "recompiled"))
        HCLOG(Debug) << message;
}

} // namespace

namespace pt
{
namespace platform
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

        // Set GL context attributes
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                            SDL_GL_CONTEXT_DEBUG_FLAG);

        // Set GL buffer attributes
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);

        // Create window
        window_ = SDL_CreateWindow(title_.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            size_.w,
            size_.h,
            SDL_WINDOW_OPENGL |
            SDL_WINDOW_ALLOW_HIGHDPI);

        // Create OpenGL context
        glContext_ = SDL_GL_CreateContext(window_);

        // Init GLAD
        const int gladStatus = gladLoadGLLoader((GLADloadproc)
                                                SDL_GL_GetProcAddress);

        if (glDebugMessageCallbackARB != nullptr)
        {
            glDebugMessageCallbackARB(debugCallback, 0);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        }

        // Set swap interval
        SDL_GL_SetSwapInterval(0);

        // Log renderer info
        HCLOG(Debug) << "OpenGL vendor: '" << glGetString(GL_VENDOR)
                     << "', renderer: '"   << glGetString(GL_RENDERER)
                     << "', version: '"    << glGetString(GL_VERSION) << "'";

        // Log GPU RAM info
        int gpuRamDedicated = 0, gpuRamTotal = 0, gpuRamAvail   = 0;
        glGetIntegerv(0x9047, &gpuRamDedicated);
        glGetIntegerv(0x9048, &gpuRamTotal);
        glGetIntegerv(0x9049, &gpuRamAvail);
        HCLOG(Debug) << "GPU RAM (dedicated, total, available): "
                     << gpuRamDedicated << ", "
                     << gpuRamTotal << ", "
                     << gpuRamAvail;

        return gladStatus;
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
    return image.flipped();
}

} // namespace ui
} // namespace pt
