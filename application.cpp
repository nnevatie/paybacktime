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
#include "texture_atlas.h"

#include "ref_mesher.h"
#include "image_mesher.h"

#include "gl_primitive.h"
#include "gl_shaders.h"
#include "gl_texture.h"
#include "gl_fbo.h"

#include "ssao.h"
#include "render_stats.h"

//#define CAPTURE_VIDEO 1

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
    Display display("High Caliber", {1280, 720});
    display.open();

    gl::Shader vsSimple(filesystem::path("shaders/simple.vs.glsl"));
    gl::Shader fsPhong(filesystem::path("shaders/phong.fs.glsl"));
    gl::Shader fsSsao(filesystem::path("shaders/ssao.fs.glsl"));
    gl::Shader fsBlur(filesystem::path("shaders/blur.fs.glsl"));
    gl::Shader gsWireframe(filesystem::path("shaders/wireframe.gs.glsl"));

    gl::ShaderProgram wireProgram({vsSimple, gsWireframe, fsPhong},
                                 {{0, "position"}, {1, "normal"}, {2, "uv"}});

    gl::ShaderProgram ssaoProgram({vsSimple, fsSsao},
                                 {{0, "position"}, {1, "normal"}, {2, "uv"}});

    gl::ShaderProgram blurProgram({vsSimple, fsBlur},
                                 {{0, "position"}, {1, "normal"}, {2, "uv"}});

    const ImageCube depthCube("data/box.*.png", 1);
    const ImageCube albedoCube("data/box.albedo.*.png");

    TextureAtlas texAtlas({128, 128});
    TextureAtlas::EntryCube albedoEntry = texAtlas.insert(albedoCube);
    texAtlas.atlas.image(true).write(filesystem::path("c:/temp/atlas.png"));

    const Mesh mesh = ImageMesher::mesh(depthCube, albedoEntry.second);
    const gl::Primitive primitive(mesh);

    const Mesh rectMesh = squareMesh();
    const gl::Primitive rectPrimitive(rectMesh);

    Ssao ssao(32, display.size(), {4, 4});

    RenderStats stats;

    int f = 0;
    bool running = true;

    float ay = 0, az = 0;

    while (running)
    {
        glm::mat4 proj  = glm::perspective(45.0f, 4.0f / 3.0f, 1.f, 200.f);
        glm::mat4 view  = glm::translate({}, glm::vec3(0.f, 0.f, -40.0f));
        glm::mat4 model = glm::rotate({}, ay, glm::vec3(0.f, 1.f, 0.f)) *
                          glm::rotate({}, az, glm::vec3(0.f, 0.f, 1.f)) *
                          glm::translate({}, glm::vec3(-8.0f, -8.0f, -8.0f));
        Clock clock;

        wireProgram.bind()
            .setUniform("albedo", 0)
            .setUniform("mvp",    proj * view * model)
            .setUniform("mv",     view * model)
            .setUniform("size",   display.size().as<glm::vec2>())
            .setUniform("color",  glm::vec4(0.1f, 0.2f, 0.4f, 1.f));
        {
            // Geometry pass
            Binder<gl::Fbo> binder(ssao.fbo[0]);
            const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                                          GL_COLOR_ATTACHMENT1};
            glDrawBuffers(2, drawBuffers);

            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            texAtlas.texture.bindAs(GL_TEXTURE0);
            primitive.render();
        }

        ssaoProgram.bind().setUniform("texColor",   0)
                          .setUniform("texNormal",  1)
                          .setUniform("texDepth",   2)
                          .setUniform("texNoise",   3)
                          .setUniform("kernel",     ssao.kernel)
                          .setUniform("noiseScale", ssao.noiseScale())
                          .setUniform("mvp",        glm::mat4())
                          .setUniform("invP",       glm::inverse(proj))
                          .setUniform("p",          proj)
                          .setUniform("r",          1.f);
        {
            // SSAO pass
            Binder<gl::Fbo> binder(ssao.fbo[1]);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glClear(GL_COLOR_BUFFER_BIT);

            ssao.texColor.bindAs(GL_TEXTURE0);
            ssao.texNormal.bindAs(GL_TEXTURE1);
            ssao.texDepth.bindAs(GL_TEXTURE2);
            ssao.texNoise.bindAs(GL_TEXTURE3);
            rectPrimitive.render();
        }

        blurProgram.bind().setUniform("texColor",  0)
                          .setUniform("texSsao",   1)
                          .setUniform("texelStep", ssao.texelStep())
                          .setUniform("mvp",       glm::mat4());
        {
            // Blur/output pass
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            ssao.texColor.bindAs(GL_TEXTURE0);
            ssao.texBlur.bindAs(GL_TEXTURE1);
            rectPrimitive.render();
        }

        #ifdef CAPTURE_VIDEO
        display.capture().write("c:/temp/f/f_" + std::to_string(f++) + ".bmp");
        a += 0.01f;
        #else
        //a += 0.001f;
        #endif

        stats.accumulate(clock.stop(), mesh.vertices.size(),
                                       mesh.indices.size() / 3);

        stats.render();
        display.swap();

        SDL_Event e;
        while (SDL_PollEvent(&e) && f < 2000)
        {
            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_LEFT)
                    ay += 0.05f;
                if (e.key.keysym.sym == SDLK_RIGHT)
                    ay -= 0.05f;
                if (e.key.keysym.sym == SDLK_UP)
                    az += 0.05f;
                if (e.key.keysym.sym == SDLK_DOWN)
                    az -= 0.05f;
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    running = false;
            }
        }
    }
    return true;
}

} // namespace
