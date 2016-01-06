#include "application.h"

#include <stdexcept>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "common/file_system.h"
#include "img/painter.h"
#include "img/image_cube.h"
#include "geom/image_mesher.h"
#include "gl/texture_atlas.h"

#include "platform/clock.h"
#include "platform/context.h"
#include "platform/display.h"
#include "platform/scheduler.h"

#include "gfx/geometry.h"
#include "gfx/ssao.h"
#include "gfx/lighting.h"
#include "gfx/bloom.h"
#include "gfx/outline.h"
#include "gfx/grid.h"
#include "gfx/color_grade.h"
#include "gfx/anti_alias.h"
#include "gfx/output.h"
#include "gfx/render_stats.h"

#include "scene/object_store.h"
#include "scene/scene.h"
#include "scene/camera.h"

namespace pt
{

Application::Application()
{
}

Application::~Application()
{
}

bool simulate(TimePoint time, Duration step)
{
    static int f = 0;
    return (f++) < 10;
}

bool render(float a)
{
    return true;
}

bool Application::run(const std::string& input)
{
    platform::Context context;
    platform::Display display("Payback Time", {1280, 720});
    display.open();

    Scheduler scheduler(std::chrono::milliseconds(10), simulate, render,
                        Scheduler::OptionPreserveCpu);
    scheduler.start();

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

    gl::TextureAtlas texAtlas({256, 256}, 2);
    gl::TextureAtlas lightAtlas({256, 256}, 2);

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

    const Size<int> renderSize(display.size());

    gfx::Geometry geometry(renderSize);
    gfx::Ssao ssao(32, renderSize, {4, 4}, geometry.texDepth);
    gfx::Lighting lighting(renderSize, geometry.texDepth);
    gfx::Bloom bloom(renderSize);
    gfx::Outline outline(renderSize, geometry.texDepth);
    gfx::Grid grid;
    gfx::ColorGrade colorGrade(renderSize);
    gfx::AntiAlias antiAlias(renderSize);
    gfx::Output output;

    gfx::RenderStats stats;

    ObjectStore objectStore(filesystem::path("objects"), &texAtlas);
    Scene scene;

    Camera camera(
        {0.f, 0.f, 0.f}, 350.f, M_PI / 2, -M_PI / 4,
        glm::radians(45.f), renderSize.aspect<float>(), 0.1, 500.f);

    int f = 0;
    float ay = 0, az = 0;

    bool running = true;
    while (running)
    {
        glm::mat4 proj  = camera.matrixProj();
        glm::mat4 view  = camera.matrixView() *
                         (glm::rotate({}, ay, glm::vec3(0.f, 1.f, 0.f)) *
                          glm::rotate({}, az, glm::vec3(1.f, 0.f, 0.f)));
        glm::mat4 model;

        Time<ChronoClock> clock;
        std::vector<gfx::Geometry::Instance> instances;

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
                instances.push_back({wall, m});
            }
        for (int y = 0; y < 5; ++y)
            for (int x = 0; x < 5; ++x)
                instances.push_back({floor,
                                     glm::translate(model,
                                        glm::vec3(x * 16, 0, y * 16))});
        for (int y = 0; y < 5; ++y)
            for (int x = 0; x < 5; ++x)
                if (y % 2 && x % 2)
                    instances.push_back({primitive,
                                         glm::translate(model,
                                             glm::vec3(x * 16, 2, y * 16))});

        geometry(&texAtlas.texture, &lightAtlas.texture,
                 instances, view, proj);

        ssao(&geometry.texDepth, &geometry.texNormalDenoise, proj, camera.fov);

        lighting(&geometry.texDepth,
                 &geometry.texNormalDenoise,
                 &geometry.texColor,
                 &geometry.texLight,
                 bloom.output(),
                 &ssao.texAoBlur,
                 &lightmap,
                 view, proj);

        bloom(&geometry.texColor, lighting.output(), &geometry.texLight);
        outline(&lighting.fbo, lighting.output(), wall, proj * view * model);
        grid(&lighting.fbo, proj * view * model);
        colorGrade(lighting.output(), bloom.output());
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
