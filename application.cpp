#include "application.h"

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
#include "gfx/backdrop.h"
#include "gfx/color_grade.h"
#include "gfx/anti_alias.h"
#include "gfx/output.h"
#include "gfx/fader.h"

#include "ui/object_selector.h"
#include "ui/render_stats.h"

#include "scene/scene.h"
#include "scene/scene_control.h"
#include "scene/camera_control.h"
#include "scene/object_store.h"

namespace pt
{

struct Impl
{
    platform::Display* display;
    Size<int> renderSize;

    gl::Texture lightmap;

    gfx::Geometry geometry;
    gfx::Ssao ssao;
    gfx::Lighting lighting;
    gfx::Bloom bloom;
    gfx::Outline outline;
    gfx::Backdrop backdrop;
    gfx::ColorGrade colorGrade;
    gfx::AntiAlias antiAlias;
    gfx::Output output;
    gfx::Fader fader;

    TextureStore textureStore;
    ObjectStore objectStore;

    ui::ObjectSelector objectSelector;
    ui::RenderStats stats;

    Scene scene;
    Camera camera;
    CameraControl cameraControl;
    SceneControl sceneControl;

    platform::Mouse mouse;

    Impl(platform::Display* display) :
        display(display),
        renderSize(display->size()),
        geometry(renderSize),
        ssao(32, renderSize, {4, 4}, geometry.texDepth),
        lighting(renderSize, geometry.texDepth),
        bloom(renderSize),
        outline(renderSize, geometry.texDepth),
        colorGrade(renderSize),
        antiAlias(renderSize),

        textureStore({256, 256}),
        objectStore(fs::path("objects"), &textureStore),

        objectSelector(display, &objectStore, &textureStore),
        stats(display->nanoVg()),

        camera({0.f, 0.f, 0.f}, 350.f, M_PI / 2, -M_PI / 4,
               glm::radians(45.f), renderSize.aspect<float>(), 0.1f, 750.f),

        cameraControl(&camera, display, &mouse),
        sceneControl(&scene, &camera, display, &mouse)
    {
        lightmap.bind().alloc(Image("data/lightmap.png"))
                       .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                       .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        textureStore.albedo.atlas.image(true).write("c:/temp/albedo.png");
        textureStore.light.atlas.image(true).write("c:/temp/light.png");
    }

    bool simulate(TimePoint /*time*/, Duration step)
    {
        mouse.reset();
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            display->processEvent(&event);
            mouse.update(event);
        }

        const uint8_t* keyState = SDL_GetKeyboardState(nullptr);
        if (keyState[SDL_SCANCODE_ESCAPE])
            return false;

        cameraControl(step);
        sceneControl(step, objectSelector.selectedObject());

        return true;
    }

    bool render(TimePoint time, float /*a*/)
    {
        const float timeSec =
            std::chrono::duration<float>(time - TimePoint()).count();

        glm::mat4 proj  = camera.matrixProj();
        glm::mat4 view  = camera.matrixView();
        glm::mat4 model;

        Time<GpuClock> clock;
        std::vector<gfx::Geometry::Instance> instances;

        gl::Primitive floor = objectStore.object("floor").model().primitive();
        gl::Primitive wall  = objectStore.object("wall").model().primitive();
        gl::Primitive box   = objectStore.object("box").model().primitive();

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
                    instances.push_back({box,
                                         glm::translate(model,
                                             glm::vec3(x * 16, 2, y * 16))});

        geometry(&textureStore.albedo.texture,
                 &textureStore.light.texture,
                 instances, view, proj);

        ssao(&geometry.texDepth, &geometry.texNormalDenoise, proj, camera.fov);

        lighting(&geometry.texDepth,
                 &geometry.texNormalDenoise,
                 &geometry.texColor,
                 &geometry.texLight,
                 &ssao.texAoBlur,
                 &lightmap,
                 view, proj);

        bloom(&geometry.texColor, lighting.output(), &geometry.texLight);

        outline(&lighting.fbo, lighting.output(), wall, proj * view * model);
        sceneControl(&lighting.fbo);

        backdrop(&lighting.fbo, camera);
        colorGrade(lighting.output(), bloom.output());
        antiAlias(colorGrade.output());
        output(antiAlias.output());

        display->renderWidgets();
        stats.accumulate(clock.elapsed(), 0, 0);
        stats();

        fader(1.f - timeSec);

        display->swap();
        return true;
    }

    bool run()
    {
        namespace arg = std::placeholders;
        Scheduler scheduler(std::chrono::milliseconds(10),
                            std::bind(&simulate, this, arg::_1, arg::_2),
                            std::bind(&render,   this, arg::_1, arg::_2),
                            Scheduler::OptionPreserveCpu);
        return scheduler.start();
    }
};

Application::Application()
{
}

bool Application::run()
{
    platform::Context context;
    platform::Display display("Payback Time", {1280, 720});
    display.open();

    return Impl(&display).run();
}

} // namespace
