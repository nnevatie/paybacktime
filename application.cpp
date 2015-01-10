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

#include "gl_mesh.h"
#include "gl_shaders.h"

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
    Display display("High Caliber", 800, 640);
    display.open();

    /*
    Image image("data/cat_life.jpg");
    Painter painter(display.surface());
    painter.drawImage(image, 0, 0);
    display.update();
    */

    gl::Shader vs(gl::Shader::Type::Vertex,   filesystem::path("data/passthrough.vs"));
    gl::Shader fs(gl::Shader::Type::Fragment, filesystem::path("data/constant.fs"));
    gl::ShaderProgram sp({vs, fs});
    sp.bind();

    const sdf::Sphere sphere(1.f);
    HCLOG(Info) << "Sphere d: " << sphere({0, 0, 0});
    HCLOG(Info) << "Sphere d: " << sphere({1, 0, 0});
    HCLOG(Info) << "Sphere d: " << sphere({2, 0, 0});

    const sdf::Box box({0.5, 0.5, 0.5});
    HCLOG(Info) << "Box d: " << box({0, 0, 0});
    HCLOG(Info) << "Box d: " << box({1, 0, 0});
    HCLOG(Info) << "Box d: " << box({2, 0, 0});
    HCLOG(Info) << "Box d: " << box({3, 0, 0});

    const Geometry boxGeometry = ReferenceExtractor::extract(box);
    const gl::Mesh mesh(boxGeometry);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0);

    float a = 0;
    bool running = true;
    while (running)
    {
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0, 0, 5);
        glRotatef(a, 0.25, 0.5, 1);
        mesh.render();
        display.swap();
        a += 0.1f;

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
