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

    gl::Shader fsCommon(filesystem::path("shaders/common.fs.glsl"));
    gl::Shader vsGeometry(filesystem::path("shaders/geometry.vs.glsl"));
    gl::Shader fsGeometry(filesystem::path("shaders/geometry.fs.glsl"));
    gl::Shader fsDenoise(filesystem::path("shaders/denoise.fs.glsl"));
    gl::Shader vsModelPos(filesystem::path("shaders/model_pos.vs.glsl"));
    gl::Shader vsQuadUv(filesystem::path("shaders/quad_uv.vs.glsl"));
    gl::Shader fsColor(filesystem::path("shaders/color.fs.glsl"));
    gl::Shader fsSsao(filesystem::path("shaders/ssao.fs.glsl"));
    gl::Shader fsBlur(filesystem::path("shaders/blur.fs.glsl"));
    gl::Shader fsLighting(filesystem::path("shaders/lighting.fs.glsl"));
    gl::Shader fsTexture(filesystem::path("shaders/texture.fs.glsl"));
    gl::Shader gsWireframe(filesystem::path("shaders/wireframe.gs.glsl"));

    gl::ShaderProgram gridProg({vsModelPos, fsColor},
                               {{0, "position"}});

    gl::ShaderProgram geomProg({vsGeometry, gsWireframe, fsGeometry, fsCommon},
                               {{0, "position"}, {1, "normal"}, {2, "uv"}});

    gl::ShaderProgram denoiseProg({vsQuadUv, fsDenoise},
                                  {{0, "position"}, {1, "uv"}});

    gl::ShaderProgram ssaoProg({vsQuadUv, fsSsao, fsCommon},
                               {{0, "position"}, {1, "uv"}});

    gl::ShaderProgram blurProg({vsQuadUv, fsBlur},
                               {{0, "position"}, {1, "uv"}});

    gl::ShaderProgram lightingProg({vsQuadUv, fsLighting, fsCommon},
                                   {{0, "position"}, {1, "uv"}});

    gl::ShaderProgram outputProg({vsQuadUv, fsTexture},
                                 {{0, "position"}, {1, "uv"}});

    const ImageCube depthCube("objects/" + input + "/*.png", 1);
    const ImageCube albedoCube("objects/" + input + "/albedo.*.png");
    const ImageCube floorCube("objects/floor/*.png", 1);
    const ImageCube floorAlb("objects/floor/albedo.*.png");
    const ImageCube wallCube("objects/wall/*.png", 1);
    const ImageCube wallAlb("objects/wall/albedo.*.png");

    gl::Texture lightmap;
    lightmap.bind().alloc(Image("data/lightmap.png"))
                   .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                   .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    TextureAtlas texAtlas({128, 128});
    TextureAtlas::EntryCube albedoEntry = texAtlas.insert(albedoCube);

    const Mesh_P_N_UV mesh = ImageMesher::mesh(depthCube, albedoEntry.second);
    const gl::Primitive primitive(mesh);

    TextureAtlas::EntryCube albedoFloor = texAtlas.insert(floorAlb);
    const Mesh_P_N_UV floorMesh = ImageMesher::mesh(floorCube, albedoFloor.second);
    const gl::Primitive floor(floorMesh);

    TextureAtlas::EntryCube albedoWall = texAtlas.insert(wallAlb);
    const Mesh_P_N_UV wallMesh = ImageMesher::mesh(wallCube, albedoWall.second);
    const gl::Primitive wall(wallMesh);

    const Mesh_P_UV rectMesh = squareMesh();
    const gl::Primitive rectPrimitive(rectMesh);

    const Mesh_P gMesh = gridMesh(16, 128, 128);
    const gl::Primitive gridPrimitive(gMesh);

    Ssao ssao(32, display.size(), {4, 4});

    RenderStats stats;

    ObjectStore objectStore(filesystem::path("objects"), &texAtlas);
    Scene scene;

    int f = 0;
    float ay = 0, az = 0;

    bool running = true;
    while (running)
    {
        const float fov = 45.f;
        const float ar  = display.size().aspect<float>();

        glm::mat4 proj  = glm::perspective(glm::radians(fov), ar, 0.1f, 400.f);
        glm::mat4 view  = glm::lookAt(glm::vec3(0.f, 200, 200),
                                      glm::vec3(0.f, 0.f, 0.f),
                                      glm::vec3(0, 1, 0)) *
                         (glm::rotate({}, ay, glm::vec3(0.f, 1.f, 0.f)) *
                          glm::rotate({}, az, glm::vec3(1.f, 0.f, 0.f)));

        glm::mat4 model;

        Clock clock;

        geomProg.bind()
            .setUniform("texAlbedo", 0)
            .setUniform("m",         model)
            .setUniform("v",         view)
            .setUniform("p",         proj)
            .setUniform("size",      display.size().as<glm::vec2>());

        {
            // Geometry pass
            Binder<gl::Fbo> binder(ssao.fboGeometry);
            const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                                          GL_COLOR_ATTACHMENT1};
            glDrawBuffers(2, drawBuffers);
            glDisable(GL_FRAMEBUFFER_SRGB);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(true);

            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            texAtlas.texture.bindAs(GL_TEXTURE0);

            for (int y = 0; y < 5; ++y)
                for (int x = 0; x < 5; ++x)
                if (y == 0 || x == 4)
                {
                    glm::mat4 m = glm::translate(model, glm::vec3(x * 16, 0, y * 16));

                    if (x == 4)
                    {
                    m = glm::translate(m, glm::vec3(16, 0, 0));
                    m = glm::rotate(m, -0.5f * float(M_PI), glm::vec3(0.f, 1.f, 0.f));
                    }

                    geomProg.bind().setUniform("m", m);
                    wall.render();
                }

            for (int y = 0; y < 5; ++y)
                for (int x = 0; x < 5; ++x)
                {
                    geomProg.bind().setUniform("m",
                        glm::translate(model, glm::vec3(x * 16, 0, y * 16)));
                    floor.render();
                }

            for (int y = 0; y < 5; ++y)
                for (int x = 0; x < 5; ++x)
                    if (y % 2 && x % 2)
                    {
                        geomProg.bind().setUniform("m",
                            glm::translate(model, glm::vec3(x * 16, 2, y * 16)));
                        primitive.render();
                    }

            denoiseProg.bind().setUniform("tex", 0);
            glDrawBuffer(GL_COLOR_ATTACHMENT2);
            glDisable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT);
            ssao.texNormal.bindAs(GL_TEXTURE0);
            rectPrimitive.render();
        }

        ssaoProg.bind().setUniform("texDepth",    0)
                       .setUniform("texNormal",   1)
                       .setUniform("texNoise",    2)
                       .setUniform("kernel",      ssao.kernel)
                       .setUniform("noiseScale",  ssao.noiseScale())
                       .setUniform("p",           proj)
                       .setUniform("tanHalfFov",  std::tan(radians(0.5f * fov)))
                       .setUniform("aspectRatio", ar);
        {
            // SSAO AO pass
            Binder<gl::Fbo> binder(ssao.fboAo);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);

            glClear(GL_COLOR_BUFFER_BIT);
            ssao.texDepth.bindAs(GL_TEXTURE0);
            ssao.texNormalDenoise.bindAs(GL_TEXTURE1);
            ssao.texNoise.bindAs(GL_TEXTURE2);
            rectPrimitive.render();
        }

        blurProg.bind().setUniform("texAo", 0);
        {
            // SSAO blur pass
            Binder<gl::Fbo> binder(ssao.fboBlur);
            glEnable(GL_DEPTH_TEST);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ssao.texAo.bindAs(GL_TEXTURE0);
            rectPrimitive.render();
        }

        lightingProg.bind().setUniform("texDepth",  0)
                           .setUniform("texNormal", 1)
                           .setUniform("texColor",  2)
                           .setUniform("texAo",     3)
                           .setUniform("texGi",     4)
                           .setUniform("v",         view)
                           .setUniform("p",         proj);
        {
            // Lighting pass
            Binder<gl::Fbo> binder(ssao.fboOutput);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(false);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ssao.texDepth.bindAs(GL_TEXTURE0);
            ssao.texNormalDenoise.bindAs(GL_TEXTURE1);
            ssao.texColor.bindAs(GL_TEXTURE2);
            ssao.texBlur.bindAs(GL_TEXTURE3);
            lightmap.bindAs(GL_TEXTURE4);
            rectPrimitive.render();
        }

        gridProg.bind()
            .setUniform("albedo", glm::vec4(0.f, 0.75f, 0.f, 1.f))
            .setUniform("mvp",    proj * view * model);
        {
            // Grid pass
            Binder<gl::Fbo> binder(ssao.fboOutput);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(true);

            gl::Texture::unbind(GL_TEXTURE_2D, GL_TEXTURE0);
            gridPrimitive.render(GL_LINES);
        }

        outputProg.bind().setUniform("texColor", 0);
        {
            // Output pass
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glEnable(GL_FRAMEBUFFER_SRGB);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(true);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ssao.texLighting.bindAs(GL_TEXTURE0);
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
