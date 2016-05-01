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

struct Data
{
    platform::Display* display;
    Size<int> renderSize;

    gfx::Geometry            geometry;
    gfx::Ssao                ssao;
    gfx::Lighting            lighting;
    gfx::Bloom               bloom;
    gfx::Outline             outline;
    gfx::Backdrop            backdrop;
    gfx::ColorGrade          colorGrade;
    gfx::AntiAlias           antiAlias;
    gfx::Output              output;
    gfx::Fader               fader;

    TextureStore             textureStore;
    ObjectStore              objectStore;

    ui::ObjectSelector       objectSelector;
    ui::RenderStats          stats;

    Scene                    scene;
    Camera                   camera;
    CameraControl            cameraControl;
    SceneControl             sceneControl;

    platform::Mouse          mouse;

    Data(platform::Display* display) :
        display(display),
        renderSize(display->size()),
        geometry(renderSize),
        ssao(32, renderSize, {4, 4}, geometry.texDepth),
        lighting(renderSize, geometry.texDepth),
        bloom(renderSize),
        outline(renderSize, geometry.texDepth),
        colorGrade(renderSize),
        antiAlias(renderSize),

        textureStore({512, 512}),
        objectStore(fs::path("objects"), &textureStore),

        objectSelector(display, &objectStore, &textureStore),
        stats(display->nanoVg()),

        camera({0.f, 0.f, 0.f}, 350.f,
               glm::half_pi<float>(), -glm::quarter_pi<float>() + 0,
               glm::radians(45.f), renderSize.aspect<float>(), 100.f, 550.f),

        cameraControl(&camera, display, &mouse),
        sceneControl(&scene, &camera, display, &mouse, geometry.texDepth)
    {

        auto floor = objectStore.object("floor");
        auto light = objectStore.object("floor3");
        auto head  = objectStore.object("head");
        auto torso = objectStore.object("torso");
        auto waist = objectStore.object("waist");
        auto thigh = objectStore.object("thigh");
        auto leg   = objectStore.object("leg");
        auto foot  = objectStore.object("foot");

        for (int y = -2; y <= 2; ++y)
            for (int x = -2; x <= 2; ++x)
            {
                auto tr = TransformTrRot({16 * x, 0, 16 * y});
                if (x || y)
                    scene.add({floor, tr});
                else
                    scene.add({light, tr});

                if (y == -2 && !x)
                {
                    auto trHead  = TransformTrRot(head.origin()  + tr.tr);
                    auto trTorso = TransformTrRot(torso.origin() + tr.tr);
                    auto trWaist = TransformTrRot(waist.origin() + tr.tr);
                    auto trThigh = TransformTrRot(thigh.origin() + tr.tr);
                    auto trLeg   = TransformTrRot(leg.origin()   + tr.tr);
                    auto trFoot  = TransformTrRot(foot.origin()  + tr.tr);

                    scene.add({head,  trHead});
                    scene.add({torso, trTorso});
                    scene.add({waist, trWaist});
                    scene.add({thigh, trThigh});
                    scene.add({leg,   trLeg});
                    scene.add({foot,  trFoot});
                }
            }
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

        const glm::mat4 proj = camera.matrixProj();
        const glm::mat4 view = camera.matrixView();

        Time<GpuClock> clock;
        geometry(&textureStore.albedo.texture,
                 &textureStore.light.texture,
                 scene.objectGeometry(Scene::GeometryType::Opaque),
                 view, proj);

        ssao(&geometry.texDepth, &geometry.texNormalDenoise, proj, camera.fov);

        lighting(&geometry.texDepth,
                 &geometry.texNormalDenoise,
                 &geometry.texColor,
                 &geometry.texLight,
                 &ssao.texAoBlur,
                 scene.lightmap(),
                 scene.incidence(),
                 scene.bounds(),
                 view, proj);

        backdrop(&lighting.fbo, camera);

        geometry(
            &lighting.fbo,
            &textureStore.albedo.texture,
            &textureStore.light.texture,
            scene.lightmap(),
            scene.incidence(),
            scene.bounds(),
            scene.objectGeometry(Scene::GeometryType::Transparent),
            camera);

        bloom(lighting.output());

        sceneControl(&lighting.fbo, lighting.output());

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
                            std::bind(&Data::simulate, this, arg::_1, arg::_2),
                            std::bind(&Data::render,   this, arg::_1, arg::_2));
        return scheduler.start();
    }
};

Application::Application()
{
}

bool Application::run(const boost::program_options::variables_map& args)
{
    platform::Context context;

    const auto fullscreen = args.count("fullscreen");
    const Size<int> size(fullscreen ? 1920 : 1280,
                         fullscreen ? 1080 : 720);

    platform::Display display("Payback Time", size, fullscreen);
    display.open();

    return Data(&display).run();
}

} // namespace
