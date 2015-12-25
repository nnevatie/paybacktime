#include "application.h"

#include <stdexcept>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "common/file_system.h"
#include "common/clock.h"

#include "img/painter.h"
#include "img/image_cube.h"

#include "geom/image_mesher.h"

#include "platform/display.h"

#include "gl/texture_atlas.h"
#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

#include "gfx/ssao.h"
#include "gfx/bloom.h"
#include "gfx/outline.h"
#include "gfx/grid.h"
#include "gfx/color_grade.h"
#include "gfx/anti_alias.h"
#include "gfx/output.h"
#include "gfx/render_stats.h"

#include "scene/object_store.h"
#include "scene/scene.h"

namespace hc
{

Application::Application()
{
}

Application::~Application()
{
}

bool Application::run(const std::string& input)
{
    platform::Display display("High Caliber", {1280, 720});
    display.open();

    gl::Shader fsCommon(gl::Shader::path("common.fs.glsl"));
    gl::Shader vsGeometry(gl::Shader::path("geometry.vs.glsl"));
    gl::Shader fsGeometry(gl::Shader::path("geometry.fs.glsl"));
    gl::Shader fsDenoise(gl::Shader::path("denoise.fs.glsl"));
    gl::Shader vsQuadUv(gl::Shader::path("quad_uv.vs.glsl"));
    gl::Shader fsLighting(gl::Shader::path("lighting.fs.glsl"));
    gl::Shader gsWireframe(gl::Shader::path("wireframe.gs.glsl"));

    gl::ShaderProgram geomProg({vsGeometry, gsWireframe, fsGeometry, fsCommon},
                              {{0, "position"}, {1, "normal"}, {2, "uv"}});

    gl::ShaderProgram denoiseProg({vsQuadUv, fsDenoise},
                                 {{0, "position"}, {1, "uv"}});

    gl::ShaderProgram lightingProg({vsQuadUv, fsLighting, fsCommon},
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

    const Size<int> renderSize(display.size());

    gfx::Ssao ssao(32, renderSize, {4, 4});
    gfx::Bloom bloom(renderSize);
    gfx::Outline outline(renderSize, ssao.texDepth);
    gfx::Grid grid;
    gfx::ColorGrade colorGrade(renderSize);
    gfx::AntiAlias antiAlias(renderSize);
    gfx::Output output;

    gfx::RenderStats stats;

    ObjectStore objectStore(filesystem::path("objects"), &texAtlas);
    Scene scene;

    int f = 0;
    float ay = 0, az = 0;

    bool running = true;
    while (running)
    {
        const float fov = 45.f;
        const float ar  = renderSize.aspect<float>();

        glm::mat4 proj  = glm::perspective(glm::radians(fov), ar, 0.1f, 500.f);
        glm::mat4 view  = glm::lookAt(glm::vec3(0.f, 250, 250),
                                      glm::vec3(0.f, 0.f, 0.f),
                                      glm::vec3(0, 1, 0)) *
                         (glm::rotate({}, ay, glm::vec3(0.f, 1.f, 0.f)) *
                          glm::rotate({}, az, glm::vec3(1.f, 0.f, 0.f)));

        glm::mat4 model;

        Time<ChronoClock> clock;

        geomProg.bind()
            .setUniform("texAlbedo", 0)
            .setUniform("texLight",  1)
            .setUniform("m",         model)
            .setUniform("v",         view)
            .setUniform("p",         proj)
            .setUniform("size",      renderSize.as<glm::vec2>());
        {
            // Geometry pass
            Binder<gl::Fbo> binder(ssao.fboGeometry);
            const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0,
                                          GL_COLOR_ATTACHMENT1,
                                          GL_COLOR_ATTACHMENT2};
            glDrawBuffers(3, drawBuffers);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
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

        ssao(proj, fov);

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
        outline(&ssao.fboOutput, &ssao.texLighting, wall, proj * view * model);
        grid(&ssao.fboOutput, proj * view * model);
        colorGrade(&ssao.texLighting, bloom.output());
        antiAlias(colorGrade.output());
        output(antiAlias.output());
        stats();

        display.swap();
        stats.accumulate(clock.elapsed(), 0, 0);

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
