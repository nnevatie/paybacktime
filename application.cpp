#include "application.h"

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/vector_query.hpp>

#include "common/file_system.h"
#include "img/image_cube.h"
#include "geom/image_mesher.h"
#include "gl/texture_atlas.h"
#include "gl/gpu_clock.h"

#include "platform/clock.h"
#include "platform/context.h"
#include "platform/display.h"
#include "platform/scheduler.h"
#include "platform/mouse.h"

#include "gfx/geometry.h"
#include "gfx/ssao.h"
#include "gfx/lighting.h"
#include "gfx/bloom.h"
#include "gfx/outline.h"
#include "gfx/grid.h"
#include "gfx/color_grade.h"
#include "gfx/anti_alias.h"
#include "gfx/output.h"

#include "ui/render_stats.h"

#include "scene/object_store.h"
#include "scene/scene.h"
#include "scene/camera_control.h"

namespace pt
{

struct Impl
{
    platform::Display* display;
    Size<int> renderSize;

    gl::TextureAtlas texAtlas;
    gl::TextureAtlas lightAtlas;

    gl::Texture lightmap;

    gfx::Geometry geometry;
    gfx::Ssao ssao;
    gfx::Lighting lighting;
    gfx::Bloom bloom;
    gfx::Outline outline;
    gfx::Grid grid;
    gfx::ColorGrade colorGrade;
    gfx::AntiAlias antiAlias;
    gfx::Output output;

    ui::RenderStats stats;

    Camera camera;
    CameraControl cameraControl;

    ImageCube depthCube;
    ImageCube albedoCube;
    ImageCube lightCube;
    ImageCube floorCube;
    ImageCube floorAlb;
    ImageCube floorLight;
    ImageCube wallCube;
    ImageCube wallAlb;
    ImageCube wallLight;

    gl::Primitive primitive,
                  floor,
                  wall;

    platform::Mouse mouse;

    float ay = 0, az = 0;

    Impl(platform::Display* display,
         const std::string& input
         ) :
        display(display),
        renderSize(display->size()),
        texAtlas({256, 256}, 2),
        lightAtlas({256, 256}, 2),

        geometry(renderSize),
        ssao(32, renderSize, {4, 4}, geometry.texDepth),
        lighting(renderSize, geometry.texDepth),
        bloom(renderSize),
        outline(renderSize, geometry.texDepth),
        colorGrade(renderSize),
        antiAlias(renderSize),
        stats(display->nanoVg()),

        camera({0.f, 0.f, 0.f}, 350.f, M_PI / 2, -M_PI / 4,
               glm::radians(45.f), renderSize.aspect<float>(), 0.1, 750.f),

        cameraControl(&camera, display, &mouse),

        depthCube("objects/" + input + "/*.png", 1),
        albedoCube("objects/" + input + "/albedo.*.png"),
        lightCube("objects/" + input + "/light.*.png"),
        floorCube("objects/floor/*.png", 1),
        floorAlb("objects/floor/albedo.*.png"),
        floorLight("objects/floor/light.*.png"),
        wallCube("objects/wall/*.png", 1),
        wallAlb("objects/wall/albedo.*.png"),
        wallLight("objects/wall/light.*.png")
    {
        lightmap.bind().alloc(Image("data/lightmap.png"))
                       .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                       .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        gl::TextureAtlas::EntryCube albedoEntry = texAtlas.insert(albedoCube);
        lightAtlas.insert(lightCube);
        lightAtlas.insert(floorLight);
        lightAtlas.insert(wallLight);

        const Mesh_P_N_UV mesh = ImageMesher::mesh(depthCube, albedoEntry.second);
        primitive = gl::Primitive(mesh);

        gl::TextureAtlas::EntryCube albedoFloor = texAtlas.insert(floorAlb);
        const Mesh_P_N_UV floorMesh = ImageMesher::mesh(floorCube, albedoFloor.second);
        floor = gl::Primitive(floorMesh);

        gl::TextureAtlas::EntryCube albedoWall = texAtlas.insert(wallAlb);
        const Mesh_P_N_UV wallMesh = ImageMesher::mesh(wallCube, albedoWall.second);
        wall = gl::Primitive(wallMesh);

        #if 0
        ObjectStore objectStore(filesystem::path("objects"), &texAtlas);
        Scene scene;
        #endif
    }

    glm::vec3 prevRayPos;
    bool simulate(TimePoint /*time*/, Duration step)
    {
        mouse.reset();
        SDL_Event event;
        while (SDL_PollEvent(&event))
            mouse.update(event);

        const uint8_t* keyState = SDL_GetKeyboardState(nullptr);
        if (keyState[SDL_SCANCODE_ESCAPE])
            return false;

        cameraControl(step);
        return true;
    }

    bool render(float /*a*/)
    {
        glm::mat4 proj  = camera.matrixProj();
        glm::mat4 view  = camera.matrixView() *
                         (glm::rotate({}, ay, glm::vec3(0.f, 1.f, 0.f)) *
                          glm::rotate({}, az, glm::vec3(1.f, 0.f, 0.f)));
        glm::mat4 model;

        Time<GpuClock> clock;
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
        grid(&lighting.fbo, &geometry.texDepth, camera);
        colorGrade(lighting.output(), bloom.output());
        antiAlias(colorGrade.output());
        output(antiAlias.output());

        stats.accumulate(clock.elapsed(), 0, 0);
        stats();

        display->swap();
        return true;
    }

    bool run()
    {
        namespace arg = std::placeholders;
        Scheduler scheduler(std::chrono::milliseconds(10),
                            std::bind(&simulate, this, arg::_1, arg::_2),
                            std::bind(&render,   this, arg::_1),
                            Scheduler::OptionPreserveCpu);
        return scheduler.start();
    }
};

Application::Application()
{
}

bool Application::run(const std::string& input)
{
    platform::Context context;
    platform::Display display("Payback Time", {1280, 720});
    display.open();

    return Impl(&display, input).run();
}

} // namespace
