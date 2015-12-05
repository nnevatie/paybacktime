#include "application.h"

#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "common/file_system.h"
#include "common/clock.h"
#include "common/log.h"

#include "img/painter.h"
#include "img/image_cube.h"

#include "geom/image_mesher.h"

#include "ui/display.h"
#include "ui/render_stats.h"

#include "gl/texture_atlas.h"
#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

#include "gfx/ssao.h"
#include "gfx/bloom.h"

#include "scene/object_store.h"
#include "scene/scene.h"

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
    ui::Display display("High Caliber", {1280, 720});
    display.open();

    gl::Shader fsCommon(gl::Shader::path("common.fs.glsl"));
    gl::Shader vsGeometry(gl::Shader::path("geometry.vs.glsl"));
    gl::Shader fsGeometry(gl::Shader::path("geometry.fs.glsl"));
    gl::Shader fsDenoise(gl::Shader::path("denoise.fs.glsl"));
    gl::Shader vsModelPos(gl::Shader::path("model_pos.vs.glsl"));
    gl::Shader vsQuadUv(gl::Shader::path("quad_uv.vs.glsl"));
    gl::Shader fsColor(gl::Shader::path("color.fs.glsl"));
    gl::Shader fsSsao(gl::Shader::path("ssao.fs.glsl"));
    gl::Shader fsBlur(gl::Shader::path("blur.fs.glsl"));
    gl::Shader fsLighting(gl::Shader::path("lighting.fs.glsl"));
    gl::Shader fsAdd(gl::Shader::path("add.fs.glsl"));
    gl::Shader fsTexture(gl::Shader::path("texture.fs.glsl"));
    gl::Shader gsWireframe(gl::Shader::path("wireframe.gs.glsl"));

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

    gl::ShaderProgram outputProg({vsQuadUv, fsAdd},
                                 {{0, "position"}, {1, "uv"}});

    const ImageCube depthCube("objects/" + input + "/*.png", 1);
    const ImageCube albedoCube("objects/" + input + "/albedo.*.png");
    const ImageCube lightCube("objects/" + input + "/light.*.png");

    const ImageCube floorCube("objects/floor/*.png", 1);
    const ImageCube floorAlb("objects/floor/albedo.*.png");
    const ImageCube floorLight("objects/floor/light.*.png");

    const ImageCube wallCube("objects/wall/*.png", 1);
    const ImageCube wallAlb("objects/wall/albedo.*.png");
    const ImageCube wallLight("objects/wall/light.*.png");

    gl::Texture lightmap;
    lightmap.bind().alloc(Image("data/lightmap.png"))
                   .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                   .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    gl::TextureAtlas texAtlas({256, 256});
    gl::TextureAtlas lightAtlas({256, 256});

    gl::TextureAtlas::EntryCube albedoEntry = texAtlas.insert(albedoCube);
    lightAtlas.insert(lightCube);
    lightAtlas.insert(floorLight);
    lightAtlas.insert(wallLight);

    const Mesh_P_N_UV mesh = ImageMesher::mesh(depthCube, albedoEntry.second);
    const gl::Primitive primitive(mesh);

    gl::TextureAtlas::EntryCube albedoFloor = texAtlas.insert(floorAlb);
    const Mesh_P_N_UV floorMesh = ImageMesher::mesh(floorCube, albedoFloor.second);
    const gl::Primitive floor(floorMesh);

    gl::TextureAtlas::EntryCube albedoWall = texAtlas.insert(wallAlb);
    const Mesh_P_N_UV wallMesh = ImageMesher::mesh(wallCube, albedoWall.second);
    const gl::Primitive wall(wallMesh);

    const Mesh_P_UV rectMesh = squareMesh();
    const gl::Primitive rectPrimitive(rectMesh);

    const Mesh_P gMesh = gridMesh(16, 128, 128);
    const gl::Primitive gridPrimitive(gMesh);

    gfx::Ssao ssao(32, display.size(), {4, 4});
    gfx::Bloom bloom(display.size());

    ui::RenderStats stats;

    ObjectStore objectStore(filesystem::path("objects"), &texAtlas);
    Scene scene;

    int f = 0;
    float ay = 0, az = 0;

    bool running = true;
    while (running)
    {
        const float fov = 45.f;
        const float ar  = display.size().aspect<float>();

        glm::mat4 proj  = glm::perspective(glm::radians(fov), ar, 0.1f, 500.f);
        glm::mat4 view  = glm::lookAt(glm::vec3(0.f, 250, 250),
                                      glm::vec3(0.f, 0.f, 0.f),
                                      glm::vec3(0, 1, 0)) *
                         (glm::rotate({}, ay, glm::vec3(0.f, 1.f, 0.f)) *
                          glm::rotate({}, az, glm::vec3(1.f, 0.f, 0.f)));

        glm::mat4 model;

        Clock clock;

        geomProg.bind()
            .setUniform("texAlbedo", 0)
            .setUniform("texLight",  1)
            .setUniform("m",         model)
            .setUniform("v",         view)
            .setUniform("p",         proj)
            .setUniform("size",      display.size().as<glm::vec2>());
        {
            // Geometry pass
            Binder<gl::Fbo> binder(ssao.fboGeometry);
            const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                                          GL_COLOR_ATTACHMENT1,
                                          GL_COLOR_ATTACHMENT2};
            glDrawBuffers(3, drawBuffers);
            glDisable(GL_FRAMEBUFFER_SRGB);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glDepthMask(true);

            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            texAtlas.texture.bindAs(GL_TEXTURE0);
            lightAtlas.texture.bindAs(GL_TEXTURE1);

            geomProg.bind();
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

                    geomProg.setUniform("m", m);
                    wall.render();
                }

            for (int y = 0; y < 5; ++y)
                for (int x = 0; x < 5; ++x)
                {
                    geomProg.setUniform("m",
                        glm::translate(model, glm::vec3(x * 16, 0, y * 16)));
                    floor.render();
                }

            for (int y = 0; y < 5; ++y)
                for (int x = 0; x < 5; ++x)
                    if (y % 2 && x % 2)
                    {
                        geomProg.setUniform("m",
                            glm::translate(model, glm::vec3(x * 16, 2, y * 16)));
                        primitive.render();
                    }

            // Denoise normals
            denoiseProg.bind().setUniform("tex", 0);
            glDrawBuffer(GL_COLOR_ATTACHMENT3);
            glDisable(GL_DEPTH_TEST);
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
            glDisable(GL_DEPTH_TEST);

            ssao.texDepth.bindAs(GL_TEXTURE0);
            ssao.texNormalDenoise.bindAs(GL_TEXTURE1);
            ssao.texNoise.bindAs(GL_TEXTURE2);
            rectPrimitive.render();
        }

        blurProg.bind().setUniform("tex", 0);
        {
            // SSAO blur pass
            Binder<gl::Fbo> binder(ssao.fboAoBlur);
            glDisable(GL_DEPTH_TEST);
            ssao.texAo.bindAs(GL_TEXTURE0);
            rectPrimitive.render();
        }

        lightingProg.bind().setUniform("texDepth",    0)
                           .setUniform("texNormal",   1)
                           .setUniform("texColor",    2)
                           .setUniform("texLight",    3)
                           .setUniform("texEmissive", 4)
                           .setUniform("texAo",       5)
                           .setUniform("texGi",       6)
                           .setUniform("v",         view)
                           .setUniform("p",         proj);
        {
            // Lighting pass
            Binder<gl::Fbo> binder(ssao.fboOutput);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(false);

            glClear(GL_DEPTH_BUFFER_BIT);
            ssao.texDepth.bindAs(GL_TEXTURE0);
            ssao.texNormalDenoise.bindAs(GL_TEXTURE1);
            ssao.texColor.bindAs(GL_TEXTURE2);
            ssao.texLight.bindAs(GL_TEXTURE3);
            bloom.output()->bindAs(GL_TEXTURE4);
            ssao.texAoBlur.bindAs(GL_TEXTURE5);
            lightmap.bindAs(GL_TEXTURE6);
            rectPrimitive.render();
        }

        bloom(&ssao.texColor, &ssao.texLighting, &ssao.texLight);

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

        outputProg.bind().setUniform("tex0", 0)
                         .setUniform("tex1", 1);
        {
            // Output pass
            glEnable(GL_FRAMEBUFFER_SRGB);
            glDisable(GL_DEPTH_TEST);

            ssao.texLighting.bindAs(GL_TEXTURE0);
            bloom.output()->bindAs(GL_TEXTURE1);
            rectPrimitive.render();
        }

        #ifdef CAPTURE_VIDEO
        display.capture().write("c:/temp/f/f_" + std::to_string(f++) + ".bmp");
        a += 0.01f;
        #else
        //a += 0.001f;
        #endif

        const int vc = 4 * mesh.vertices.size() +
                       5 * 5 * floorMesh.vertices.size() +
                       10 * wallMesh.vertices.size();
        const int ic = (4 * mesh.indices.size() +
                        5 * 5 * floorMesh.indices.size() +
                        10 * wallMesh.indices.size()) / 3;

        stats.render();
        display.swap();

        stats.accumulate(clock.stop(), vc, ic);

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
