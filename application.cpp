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
#include "gfx/ssr.h"
#include "gfx/lighting.h"
#include "gfx/bloom.h"
#include "gfx/outline.h"
#include "gfx/backdrop.h"
#include "gfx/color_grade.h"
#include "gfx/anti_alias.h"
#include "gfx/output.h"
#include "gfx/fader.h"

#include "ui/render_stats.h"
#include "ui/tools_window.h"
#include "ui/scene_pane.h"
#include "ui/object_pane.h"

#include "scene/scene.h"
#include "scene/scene_control.h"
#include "scene/camera_control.h"
#include "scene/object_store.h"
#include "scene/horizon_store.h"

namespace pt
{

struct Data
{
    platform::Display* display;
    Size<int>          renderSize;

    gfx::Geometry      geometry;
    gfx::Ssao          ssao;
    gfx::Ssr           ssr;
    gfx::Lighting      lighting;
    gfx::Bloom         bloom;
    gfx::Outline       outline;
    gfx::Backdrop      backdrop;
    gfx::ColorGrade    colorGrade;
    gfx::AntiAlias     antiAlias;
    gfx::Output        output;
    gfx::Fader         fader;

    TextureStore       textureStore;
    ObjectStore        objectStore;
    HorizonStore       horizonStore;
    Character          character;

    Scene              scene;
    Camera             camera;
    CameraControl      cameraControl;
    SceneControl       sceneControl;

    ui::RenderStats    stats;
    ui::ToolsWindow    toolsWindow;
    ui::ScenePane      scenePane;
    ui::ObjectPane     objectPane;

    platform::Mouse    mouse;

    Data(platform::Display* display) :
        display(display),
        renderSize(display->size()),
        geometry(renderSize),
        ssao(32, renderSize, {4, 4}),
        ssr(renderSize),
        lighting(renderSize, geometry.texDepth),
        bloom(renderSize),
        outline(renderSize, geometry.texDepth),
        colorGrade(renderSize),
        antiAlias(renderSize),

        textureStore({512, 512}),
        objectStore(fs::path("objects"), &textureStore),
        horizonStore(fs::path("horizons")),
        character(fs::path("characters") / "male1", &textureStore),

        // Camera
        camera({0.f, 0.f, 0.f}, 450.f,
               glm::half_pi<float>(), -glm::quarter_pi<float>() + 0,
               glm::radians(45.f), renderSize.aspect<float>(), 100.f, 750.f),

        // Controls
        cameraControl(&camera, display, &mouse),
        sceneControl(&scene, &camera, display, &mouse, geometry.texDepth),

        // UI
        stats(display->nanoVg()),
        toolsWindow(display),
        scenePane(toolsWindow.add("Scene"), display, &scene, &horizonStore),
        objectPane(toolsWindow.add("Objects", true),
                   display, &objectStore, &textureStore)
    {
        #if 0
        auto floor = objectStore.object("floor");
        auto light = objectStore.object("floor3");

        #if 0
        auto pillar = objectStore.object("pillar");
        scene.add({pillar, TransformTrRot({0, 0, 0})});
        #endif

        for (int y = -5; y <= 5; ++y)
            for (int x = -5; x <= 5; ++x)
            {
                auto tr = TransformTrRot({16 * x, -2, 16 * y});
                scene.add({x || y ? floor : light, tr});
            }

        #if 0
        for (int y = -20; y <= 20; ++y)
            for (int x = -20; x <= 20; ++x)
            {
                auto tr = TransformTrRot({16 * x, -2, 16 * y});
                scene.add({light, tr});
            }
        #endif

        #if 0
        scene.add({objectStore.object("chair"), TransformTrRot({-32, 0, 16 * -2})});
        scene.add({objectStore.object("table"), TransformTrRot({32, 0, 16 * -2})});
        #endif
        #endif

        toolsWindow.select(1);
        display->update();
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
        sceneControl(step, objectPane.selected());

        return true;
    }

    bool render(TimePoint time, float /*a*/)
    {
        const float timeSec =
            boost::chrono::duration<float>(time - TimePoint()).count();

        const glm::mat4 proj = camera.matrixProj();
        const glm::mat4 view = camera.matrixView();

        TimeTree<GpuClock> timeTree;
        auto timeTotal = timeTree.scope("total");

        const gfx::Geometry::Instances chars =
            scene.characterGeometry();

        gfx::Geometry::Instances geom =
            scene.objectGeometry(Scene::GeometryType::Opaque);
        geom.insert(geom.end(), chars.begin(), chars.end());

        {
            auto time = timeTree.scope("geom-opq");
            geometry(&textureStore.albedo.texture,
                     &textureStore.normal.texture,
                     &textureStore.light.texture,
                     geom, view, proj);
        }
        {
            auto time = timeTree.scope("ssao");
            ssao(&geometry.texDepth, &geometry.texNormalDenoise,
                 proj, camera.fov);
        }
        {
            auto time = timeTree.scope("lighting");
            lighting(&geometry.texDepth,
                     &geometry.texNormalDenoise,
                     &geometry.texColor,
                     &geometry.texLight,
                     &ssao.texAoBlur,
                     scene.lightmap(),
                     scene.incidence(),
                     camera,
                     scene.bounds(),
                     view, proj);
        }
        {
            auto time = timeTree.scope("backdrop");
            backdrop(&lighting.fbo, camera);
        }
        {
            auto time = timeTree.scope("geom-tr");
            geometry(
                &lighting.fbo,
                &textureStore.albedo.texture,
                &textureStore.light.texture,
                scene.lightmap(),
                scene.incidence(),
                scene.bounds(),
                scene.objectGeometry(Scene::GeometryType::Transparent),
                camera);
        }
        {
            auto time = timeTree.scope("ssr");
            ssr(&geometry.texDepth, &geometry.texNormalDenoise, lighting.output(),
               &geometry.texLight, camera);
        }
        {
            auto time = timeTree.scope("bloom");
            bloom(ssr.output());
        }

        sceneControl(&ssr.fboSsr, ssr.output());

        {
            auto time = timeTree.scope("colorgrade");
            colorGrade(ssr.output(), bloom.output());
        }
        {
            auto time = timeTree.scope("anti-alias");
            antiAlias(colorGrade.output());
        }
        {
            auto time = timeTree.scope("output");
            output(antiAlias.output());
        }
        {
            auto time = timeTree.scope("ui");
            display->renderWidgets();
        }

        timeTotal.end();
        stats.accumulate(timeTree);
        stats(scene.cellResolution());

        fader(1.f - timeSec);

        display->swap();
        return true;
    }

    bool run()
    {
        namespace arg = std::placeholders;
        Scheduler scheduler(boost::chrono::milliseconds(10),
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
    const Size<int> size(1920, 1080);

    platform::Display display("Payback Time", size, fullscreen);
    display.open();

    return Data(&display).run();
}

} // namespace
