#include "application.h"

#include <boost/type_traits/is_assignable.hpp>
#include <boost/format.hpp>

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
#include "gfx/mipmap.h"
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

#include "common/config.h"

namespace pt
{

struct Data
{
    platform::Display* display;
    cfg::Config        config;

    Size<int>          renderSize;

    gfx::Geometry      geometry;
    gfx::Ssao          ssao;
    gfx::Ssr           ssr;
    gfx::Lighting      lighting;
    gfx::Bloom         bloom;
    gfx::Outline       outline;
    gfx::Backdrop      backdrop;
    gfx::Mipmap        envMipmap;
    gfx::ColorGrade    colorGrade;
    gfx::AntiAlias     antiAlias;
    gfx::Output        output;
    gfx::Fader         fader;

    TextureStore       textureStore;
    ObjectStore        objectStore,
                       characterStore;
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

    TimePoint          lastLiveUpdate;

    ThroughputCpu      throughput;

    Data(platform::Display* display, const cfg::Config& config) :
        display(display),
        config(config),
        renderSize(display->size() * config.video.output.scale),
        geometry(renderSize),
        ssao(config.video.ssao.samples,
             renderSize, renderSize * config.video.ssao.scale),
        ssr(renderSize, renderSize * config.video.ssr.scale),
        lighting(config.video, geometry.texDepth),
        bloom(renderSize),
        outline(renderSize, geometry.texDepth),
        backdrop(renderSize),
        envMipmap(renderSize * config.video.env.scale, 4, true),
        colorGrade(renderSize),
        antiAlias(renderSize),
        output(display->size()),

        textureStore({1024, 1024}),
        objectStore(fs::path("objects"), textureStore),
        characterStore(fs::path("characters"), textureStore),
        horizonStore(fs::path("horizons")),
        character("male1", characterStore, textureStore),

        // Camera
        camera({0.f, 0.f, 0.f}, 400.f,
               glm::half_pi<float>(), -glm::quarter_pi<float>(),
               glm::radians(45.f), renderSize.aspect<float>(), 100.f, 750.f),

        // Controls
        cameraControl(&camera, display, &mouse),
        sceneControl(&scene, &camera, display, &mouse,
                     renderSize, geometry.texDepth),

        // UI
        stats(display->nanoVg()),
        toolsWindow(display),
        scenePane(toolsWindow.add("Scene"), display, &scene,
                  &horizonStore, &objectStore),
        objectPane(toolsWindow.add("Objects", true),
                   display, &objectStore, &textureStore)
    {
        toolsWindow.select(1);
        display->update();
    }

    bool simulate(TimePoint time, Duration step)
    {
        // Process events
        mouse.reset();
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            display->processEvent(&event);
            mouse.update(event);
        }

        // Read key states
        const uint8_t* keyState = SDL_GetKeyboardState(nullptr);
        if (keyState[SDL_SCANCODE_ESCAPE])
            return false;

        // Camera control
        cameraControl(step);

        // Scene control
        sceneControl(time, step, objectPane.selected());

        // Animate scene
        scene.animate(time, step);

        // Update object store
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
            time - lastLiveUpdate).count() > 1000)
        {
            if (objectStore.update(textureStore))
                scene.updateLightmap();

            characterStore.update(textureStore);
            lastLiveUpdate = time;
        }

        // Add character
        if (scene.characterGeometry().empty())
        {
            scene.add(CharacterItem(character, {glm::vec3(-80, 0, -48)}));
            scene.updateLightmap();
        }

        // UI actions
        if (auto action = scenePane.nextAction())
            action();

        return true;
    }

    bool render(TimePoint time, float /*a*/)
    {
        const auto timeSec = std::chrono::duration<float>(time.time_since_epoch()).count();
        const auto detailedStats = config.debug.detailedStats;

        TimeTree<GpuClock> timeTree;
        auto timeTotal = timeTree.scope("total", detailedStats);

        const gfx::Geometry::Instances chars =
            scene.characterGeometry();

        gfx::Geometry::Instances geom =
            scene.objectGeometry(Scene::GeometryType::Opaque);
        geom.insert(geom.end(), chars.begin(), chars.end());

        {
            auto time = timeTree.scope("geom-opq", detailedStats);
            geometry(&textureStore.albedo.texture,
                     &textureStore.normal.texture,
                     &textureStore.light.texture,
                     geom, camera);
        }
        {
            auto time = timeTree.scope("ssao", detailedStats);
            ssao(&geometry.texDepthLinear,
                 &geometry.texNormalDenoise,
                 camera.matrixProj(), camera.fov);
        }
        {
            auto time = timeTree.scope("lighting-sc", detailedStats);
            lighting.sc(&geometry.texDepth,
                        &scene.lightmap().light().second,
                        camera,
                        scene.bounds());
        }
        {
            auto time = timeTree.scope("lighting", detailedStats);
            lighting(&geometry.texDepth,
                     &geometry.texNormalDenoise,
                     &geometry.texColor,
                     &geometry.texLight,
                     &ssao.output(),
                     &scene.lightmap().light().second,
                     &scene.lightmap().incidence().second,
                     camera,
                     scene.bounds(),
                     timeSec);
        }
        {
            auto time = timeTree.scope("env-mips", detailedStats);
            envMipmap(lighting.output(), &geometry.texDepth);
        }
        {
            auto time = timeTree.scope("backdrop", detailedStats);
            backdrop(&lighting.fboOut, camera);
        }
        {
            auto time = timeTree.scope("ssr", detailedStats);
            ssr(&geometry.texDepthLinear,
                &geometry.texNormalDenoise,
                &geometry.texLight,
                lighting.output(),
                envMipmap.output(),
                camera);
        }
        {
            auto time = timeTree.scope("geom-tr", detailedStats);
            geometry(
                ssr.output(),
                envMipmap.output(),
                &textureStore.albedo.texture,
                &textureStore.light.texture,
                &scene.lightmap().light().second,
                &scene.lightmap().incidence().second,
                scene.bounds(),
                scene.objectGeometry(Scene::GeometryType::Transparent),
                camera);
        }
        {
            auto time = timeTree.scope("bloom", detailedStats);
            bloom(&geometry.texComp);
        }
        {
            auto time = timeTree.scope("scene-ctrl", detailedStats);
            sceneControl(&geometry.fboComp, &geometry.texComp);
        }
        {
            auto time = timeTree.scope("colorgrade", detailedStats);
            colorGrade(&geometry.texComp, bloom.output());
        }
        {
            auto time = timeTree.scope("anti-alias", detailedStats);
            antiAlias(colorGrade.output());
        }
        {
            auto time = timeTree.scope("output", detailedStats);
            output(antiAlias.output());
            #if 0
            output(&scene.lightmap().debug(&geometry.texDepth,
                                           renderSize,
                                           camera,
                                           scene.bounds()));
            #endif
        }
        {
            auto time = timeTree.scope("ui", detailedStats);
            display->renderWidgets();
        }

        timeTotal.end();

        stats.accumulate(timeTree);
        stats(throughput(), scene.cellResolution());

        fader(1.f - timeSec);

        display->swap();

        #if 0
        static int f = 0;
        if (timeSec > 10.f)
        {
            auto cap = display->capture().scaled({1920, 1080});
            boost::format fmt("%05d");
            fmt % (f++);
            auto fn = "c:/temp/pt/" + fmt.str() + ".png";
            cap.write(fn);
            camera.yaw += 0.01f;
        }
        #endif

        return true;
    }

    bool run()
    {
        namespace arg = std::placeholders;
        Scheduler scheduler(std::chrono::milliseconds(20),
                            std::bind(&Data::simulate, this, arg::_1, arg::_2),
                            std::bind(&Data::render,   this, arg::_1, arg::_2));
        return scheduler.start();
    }
};

bool Application::run(const boost::program_options::variables_map& args)
{
    platform::Context context;

    const auto fullscreen = bool(args.count("fullscreen"));
    const Size<int> size(1920, 1080);

    platform::Display display("Payback Time - Scene Editor", size, fullscreen,
                              Image(fs::path("data/paybacktime.png")));
    display.open();

    auto config = cfg::preset::config;
    config.video.output.size = {size.w, size.h};

    return Data(&display, config).run();
}

} // namespace
