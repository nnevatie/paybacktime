#include "application.h"

#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "file_system.h"

#include "display.h"
#include "painter.h"
#include "clock.h"
#include "log.h"

#include "sdf_primitives.h"
#include "image_cube.h"

#include "ref_mesher.h"
#include "image_mesher.h"

#include "gl_mesh.h"
#include "gl_shaders.h"

#include "render_stats.h"

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
    Display display("High Caliber", 1280, 720);
    display.open();

    gl::Shader vsSimple(gl::Shader::Type::Vertex,
                        filesystem::path("data/simple.vs"));

    gl::Shader fsScreenspace(gl::Shader::Type::Fragment,
                             filesystem::path("data/screenspace.fs"));

    gl::Shader gsWireframe(gl::Shader::Type::Geometry,
                           filesystem::path("data/wireframe.gs"));

    gl::ShaderProgram wireProgram({vsSimple, gsWireframe, fsScreenspace});

    const ImageCube geomSrc("data/floor.*.png", 1);
    //const Image geomSrc("data/floor.top.png", 1);

    const Geometry geom = ImageMesher::geometry(geomSrc);
    const gl::Mesh mesh(geom);

    RenderStats stats;

    float a = 1.2;
    bool running = true;

    while (running)
    {
        Clock clock;

        glm::mat4 proj  = glm::perspective(45.0f, 4.0f / 3.0f, 0.01f, 400.f);
        glm::mat4 view  = glm::translate(glm::mat4(),
                                         glm::vec3(-8.0f, -2.0f, -40.0f));

        glm::mat4 model = glm::rotate(glm::mat4(), a, glm::vec3(1.0f, 0.5f, 0.9f));
        glm::mat4 mvp   = proj * view * model;

        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        wireProgram.bind()
            .setUniform("transform", mvp)
            .setUniform("winSize",   glm::vec2(display.width(), display.height()))
            .setUniform("color",     glm::vec4(0.1f, 0.2f, 0.4f, 1.f));

        mesh.render();

        stats.accumulate(clock.stop(), geom.vertices.size(),
                                       geom.indices.size() / 3);

        stats.render();
        display.swap();

        a += 0.001f;

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
