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
#include "reference_extractor.h"
#include "image_cube.h"

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

    Image image("data/cat_life.jpg");
    ImageCube cube(
        {image, image, image, image, image, image}
    );

    gl::Shader vsSimple(gl::Shader::Type::Vertex,
                        filesystem::path("data/simple.vs"));

    gl::Shader fsScreenspace(gl::Shader::Type::Fragment,
                             filesystem::path("data/screenspace.fs"));

    gl::Shader fsConstant(gl::Shader::Type::Fragment,
                          filesystem::path("data/constant.fs"));

    gl::ShaderProgram fillProgram({vsSimple, fsScreenspace});
    gl::ShaderProgram wireProgram({vsSimple, fsConstant});

    const sdf::Box box({0.5, 0.5, 0.5});
    const Geometry boxGeometry = ReferenceExtractor::extract(box);
    const gl::Mesh mesh(boxGeometry);

    glEnable(GL_CULL_FACE);

    float a = 0;
    bool running = true;
    while (running)
    {
        glm::mat4 proj  = glm::perspective(45.0f, 4.0f / 3.0f, 0.01f, 6.f);
        glm::mat4 view  = glm::translate(glm::mat4(),
                                         glm::vec3(0.0f, 0.0f, -2.0f));

        glm::mat4 model = glm::rotate(glm::mat4(), a, glm::vec3(1.f, 0.5f, 0.1f));
        glm::mat4 mvp = proj * view * model;

        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        fillProgram.bind()
            .setUniform("in_matrix", mvp)
            .setUniform("in_color", glm::vec4(0.1f, 0.2f, 0.4f, 1.f));

        glEnable(GL_DEPTH_TEST);
        mesh.render();

        wireProgram.bind()
            .setUniform("in_matrix", mvp)
            .setUniform("in_color", glm::vec4(1.0f, 1.0f, 1.0f, 1.f));

        glDisable(GL_DEPTH_TEST);
        mesh.render(gl::Mesh::RenderType::Lines);

        display.swap();
        a += 0.005f;

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