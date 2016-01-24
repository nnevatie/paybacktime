#include "display.h"

#include <glad/glad.h>

#include <nanovg.h>
#include <nanovg_gl.h>

#include <include/screen.h>

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

struct Display::Data
{
    Data(const std::string& title, const Size<int>& size) :
        title(title), size(size),
        window(nullptr), glContext(nullptr),
        nvgContext(nullptr), nanoguiScreen(nullptr)
    {
    }
    ~Data()
    {
    }

    std::string      title;
    Size<int>        size;
    SDL_Window*      window;
    SDL_GLContext    glContext;
    NVGcontext*      nvgContext;
    nanogui::Screen* nanoguiScreen;
};

Display::Display(const std::string& title, const Size<int>& size) :
    d(new Data(title, size))
{
}

Display::~Display()
{
    close();
}

Size<int> Display::size() const
{
    return d->size;
}

SDL_Window* Display::window() const
{
    return d->window;
}

SDL_Surface* Display::surface() const
{
    return SDL_GetWindowSurface(d->window);
}

NVGcontext* Display::nanoVg() const
{
    return d->nvgContext;
}

nanogui::Screen*Display::nanoGui() const
{
    return d->nanoguiScreen;
}

bool Display::open()
{
    HCTIME("");
    if (!d->window)
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
                            SDL_GL_CONTEXT_DEBUG_FLAG |
                            SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

        // Set GL buffer attributes
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        // Create window
        d->window = SDL_CreateWindow(d->title.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            d->size.w,
            d->size.h,
            SDL_WINDOW_OPENGL |
            SDL_WINDOW_ALLOW_HIGHDPI);

        // Create OpenGL context
        d->glContext = SDL_GL_CreateContext(d->window);

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

        // Clear screen
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(d->window);

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

        // Init NanoVG
        d->nvgContext = nvgCreateGL3(0);

        // Init NanoGUI
        d->nanoguiScreen = new nanogui::Screen(d->nvgContext, d->window);
        return gladStatus;
    }
    else
        HCLOG(Warn) << "Window already open.";

    return false;
}

bool Display::close()
{
    if (d->window)
    {
        SDL_GL_DeleteContext(d->glContext);
        SDL_DestroyWindow(d->window);
        nvgDeleteGL3(d->nvgContext);
        delete d->nanoguiScreen;

        d->glContext     = nullptr;
        d->window        = nullptr;
        d->nvgContext    = nullptr;
        d->nanoguiScreen = nullptr;
        return true;
    }
    else
        HCLOG(Warn) << "Window not open.";

    return false;
}

bool Display::update()
{
    return !SDL_UpdateWindowSurface(d->window);
}

bool Display::swap()
{
    if (d->window)
    {
        SDL_GL_SwapWindow(d->window);
        return true;
    }
    return false;
}

glm::vec3 Display::rayNdc(const glm::ivec2& p) const
{
    return glm::vec3((2.f * p.x) / d->size.w - 1.f,
                     1.f - (2.f * p.y) / d->size.h, 1.f);
}

glm::vec4 Display::rayClip(const glm::ivec2& p) const
{
    const glm::vec3 n = rayNdc(p);
    return glm::vec4(n.x, n.y, -1.f, 1.f);
}

Display& Display::processEvent(SDL_Event* event)
{
    if (event) d->nanoguiScreen->onEvent(*event);
    return *this;
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
