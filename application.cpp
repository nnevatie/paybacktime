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

#include "image_cube.h"
#include "texture_atlas.h"
#include "image_mesher.h"

#include "gl_primitive.h"
#include "gl_shaders.h"
#include "gl_texture.h"
#include "gl_fbo.h"

#include "ssao.h"
#include "render_stats.h"

#include "object_store.h"
#include "scene.h"

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

bool Application::run(const std::string& input)
{
    Display display("High Caliber", {1280, 720});
    display.open();

    gl::Shader vsGeometry(filesystem::path("shaders/geometry.vs.glsl"));
    gl::Shader fsGeometry(filesystem::path("shaders/geometry.fs.glsl"));
    gl::Shader vsTexture(filesystem::path("shaders/texture.vs.glsl"));
    gl::Shader fsColor(filesystem::path("shaders/color.fs.glsl"));
    gl::Shader fsSsao(filesystem::path("shaders/ssao.fs.glsl"));
    gl::Shader fsBlur(filesystem::path("shaders/blur.fs.glsl"));
    gl::Shader gsWireframe(filesystem::path("shaders/wireframe.gs.glsl"));

    //gl::ShaderProgram gridProg({vsSimple, fsColor},
    //                           {{0, "position"}});

    gl::ShaderProgram geomProg({vsGeometry, /*gsWireframe,*/ fsGeometry},
                               {{0, "position"}, {1, "normal"}, {2, "uv"}});

    gl::ShaderProgram ssaoProg({vsTexture, fsSsao},
                               {{0, "position"}, {1, "uv"}});

    gl::ShaderProgram blurProg({vsTexture, fsBlur},
                               {{0, "position"}, {1, "uv"}});

    const ImageCube depthCube("objects/" + input + ".*.png", 1);
    const ImageCube albedoCube("objects/" + input + ".albedo.*.png");

    TextureAtlas texAtlas({128, 128});
    TextureAtlas::EntryCube albedoEntry = texAtlas.insert(albedoCube);

    const Mesh_P_N_UV mesh = ImageMesher::mesh(depthCube, albedoEntry.second);
    const gl::Primitive primitive(mesh);

    const Mesh_P_UV rectMesh = squareMesh();
    const gl::Primitive rectPrimitive(rectMesh);

    const Mesh_P gMesh = gridMesh(16, 128, 128);
    const gl::Primitive gridPrimitive(gMesh);

    Ssao ssao(32, display.size(), {4, 4});

    RenderStats stats;

    /*
    ObjectStore objectStore(filesystem::path("objects"), &texAtlas);
    Scene scene;
    */

    int f = 0;
    float ay = 0, az = 0;

    bool running = true;
    while (running)
    {
        glm::mat4 proj  = glm::perspective(45.0f, display.size().aspect<float>(),
                                           0.1f, 200.f);

        glm::mat4 view  = glm::lookAt(glm::vec3(0.f, 40, 40),
                                      glm::vec3(0.f, 0.f, 0.f),
                                      glm::vec3(0, 1, 0));

        glm::mat4 model = glm::rotate({}, ay, glm::vec3(0.f, 1.f, 0.f)) *
                          glm::rotate({}, az, glm::vec3(0.f, 0.f, 1.f));
        Clock clock;
        {
            // Geometry pass
            Binder<gl::Fbo> binder(ssao.fboGeometry);
            const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                                          GL_COLOR_ATTACHMENT1,
                                          GL_COLOR_ATTACHMENT2};
            glDrawBuffers(3, drawBuffers);

            glEnable(GL_DEPTH_TEST);
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/*
            gridProg.bind()
                .setUniform("albedo", glm::vec4(0, 0.5f, 0, 1))
                .setUniform("mvp",    proj * view * model)
                .setUniform("mv",     view * model)
                .setUniform("size",   display.size().as<glm::vec2>());

            gl::Texture::unbind(GL_TEXTURE_2D, GL_TEXTURE0);
            gridPrimitive.render(GL_LINES);
*/
            geomProg.bind()
                .setUniform("albedo", 0)
                .setUniform("mv",     view * model)
                .setUniform("p",      proj)
                .setUniform("size",   display.size().as<glm::vec2>());

            texAtlas.texture.bindAs(GL_TEXTURE0);
            primitive.render();
        }

        ssaoProg.bind().setUniform("texPosDepth", 0)
                       .setUniform("texNormal",   1)
                       .setUniform("texNoise",    2)
                       .setUniform("kernel",      ssao.kernel)
                       .setUniform("noiseScale",  ssao.noiseScale())
                       .setUniform("p",           proj);
        {
            // SSAO AO pass
            Binder<gl::Fbo> binder(ssao.fboAo);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glClear(GL_COLOR_BUFFER_BIT);

            ssao.texPosDepth.bindAs(GL_TEXTURE0);
            ssao.texNormal.bindAs(GL_TEXTURE1);
            ssao.texNoise.bindAs(GL_TEXTURE2);
            rectPrimitive.render();
        }

        blurProg.bind().setUniform("texColor", 0)
                       .setUniform("texAo",    1);
        {
            // SSAO blur pass
            // TODO: Direct to FBO
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            ssao.texColor.bindAs(GL_TEXTURE0);
            ssao.texAo.bindAs(GL_TEXTURE1);
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
