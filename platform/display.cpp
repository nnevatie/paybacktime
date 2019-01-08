#include "display.h"

#include <glad/glad.h>

#include <nanovg.h>
#include <nanovg_gl.h>

#include <nanogui/screen.h>

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
         severity == GL_DEBUG_SEVERITY_HIGH_ARB))
        PTLOG(Debug) << message;
}

} // namespace

namespace pt
{
namespace platform
{

struct Display::Data
{
    Data(const std::string& title, const Size<int>& size, bool fullscreen,
         const Image& icon) :
        title(title), size(size), fullscreen(fullscreen), icon(icon),
        window(nullptr), glContext(nullptr),
        nvgContext(nullptr), nanoGuiScreen(nullptr)
    {}

    std::string      title;
    Size<int>        size;
    bool             fullscreen;
    Image            icon;
    SDL_Window*      window;
    SDL_GLContext    glContext;
    NVGcontext*      nvgContext;
    nanogui::Screen* nanoGuiScreen;
};

Display::Display(const std::string& title,
                 const Size<int>& size,
                 bool fullscreen,
                 const Image& icon) :
    d(std::make_shared<Data>(title, size, fullscreen, icon))
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
    return d->nanoGuiScreen;
}

bool Display::open()
{
    PTTIME("");
    if (!d->window)
    {
        // Log available video drivers
        PTLOG(Debug) << "Video drivers:";
        const int driverCount = SDL_GetNumVideoDrivers();
        for (int i = 0; i < driverCount; ++i)
            PTLOG(Debug) << "#" << (i + 1) << ": "
                         << SDL_GetVideoDriver(i);

        // Set GL context attributes
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
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
           (d->fullscreen ? SDL_WINDOW_FULLSCREEN : 0) |
            SDL_WINDOW_OPENGL |
            SDL_WINDOW_ALLOW_HIGHDPI);

        // Set icon
        if (d->icon)
            SDL_SetWindowIcon(d->window, d->icon.surface());

        // Create OpenGL context
        d->glContext = SDL_GL_CreateContext(d->window);

        // Init GLAD
        gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);

        //gladLoadGL();

        /*
        // Init glbinding
        glbinding::GetProcAddress resolver;
        glbinding::Binding::initialize(resolver);

        // Error logging
        glbinding::Binding::setCallbackMaskExcept(glbinding::CallbackMask::After,
                                                 {"glGetError"});
        glbinding::Binding::setAfterCallback([](const glbinding::FunctionCall &)
        {
            const auto error = glGetError();
            if (error != GL_NO_ERROR)
                PTLOG(Error) << "GL error: "
                             << std::hex << int(error) << std::endl;
        });
        */

        // Debug callback
        glDebugMessageCallbackARB(debugCallback, 0);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

        // Set swap interval
        SDL_GL_SetSwapInterval(0);

        // Clear screen
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_FRAMEBUFFER_SRGB);
        SDL_GL_SwapWindow(d->window);

        // Log renderer info
        PTLOG(Debug) << "OpenGL vendor: '" << glGetString(GL_VENDOR)
                     << "', renderer: '"   << glGetString(GL_RENDERER)
                     << "', version: '"    << glGetString(GL_VERSION) << "'";

        // Log GPU RAM info
        GLint gpuRamDedicated = 0, gpuRamTotal = 0, gpuRamAvail = 0;
        glGetIntegerv(GLenum(0x9047), &gpuRamDedicated);
        glGetIntegerv(GLenum(0x9048), &gpuRamTotal);
        glGetIntegerv(GLenum(0x9049), &gpuRamAvail);
        PTLOG(Debug) << "GPU RAM (dedicated, total, available): "
                     << gpuRamDedicated << ", "
                     << gpuRamTotal << ", "
                     << gpuRamAvail;

        // Init NanoVG
        d->nvgContext = nvgCreateGL3(0);

        // Init NanoGUI
        d->nanoGuiScreen = new nanogui::Screen(d->nvgContext, d->window);
        return true;
    }
    else
        PTLOG(Warn) << "Window already open.";

    return false;
}

bool Display::close()
{
    if (d->window)
    {
        delete d->nanoGuiScreen;
        nvgDeleteGL3(d->nvgContext);
        SDL_GL_DeleteContext(d->glContext);
        SDL_DestroyWindow(d->window);

        d->glContext     = nullptr;
        d->window        = nullptr;
        d->nvgContext    = nullptr;
        d->nanoGuiScreen = nullptr;
        return true;
    }
    else
        PTLOG(Warn) << "Window not open.";

    return false;
}

bool Display::update()
{
    nanoGui()->performLayout();
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

glm::vec3 Display::ndc(const glm::ivec2& p) const
{
    return glm::vec3((2.f * p.x) / d->size.w - 1.f,
               1.f - (2.f * p.y) / d->size.h, 1.f);
}

glm::vec4 Display::clip(const glm::ivec2& p) const
{
    const glm::vec3 n = ndc(p);
    return glm::vec4(n.x, n.y, -1.f, 1.f);
}

Display& Display::processEvent(SDL_Event* event)
{
    if (event) d->nanoGuiScreen->onEvent(*event);
    return *this;
}

Display& Display::renderWidgets()
{
    d->nanoGuiScreen->drawAll();
    return *this;
}

Image Display::capture() const
{
    const Size<int> size = this->size();
    const int stride     = size.w * 4;

    Image image(size, 4, stride);
    glReadPixels(0, 0, size.w, size.h, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    return image.flipped(Image::Axis::X);
}

} // namespace ui
} // namespace pt
