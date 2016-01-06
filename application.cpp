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
#include "gfx/render_stats.h"

#include "scene/object_store.h"
#include "scene/scene.h"
#include "scene/camera.h"

namespace pt
{

struct CameraControl
{
    // Velocity & acceleration vectors
    glm::vec3 pos[2], ang[2];

    CameraControl& operator()(Camera* camera, Duration step)
    {
        using namespace glm;

        const float t   = std::chrono::duration<float>(step).count();
        const vec3 forw = normalize(vec3(camera->forward().x,
                                         0,
                                         camera->forward().z));
        // Position
        pos[0]         += t * pos[1];
        camera->target += t * (camera->right() * pos[0].x + forw * pos[0].y);
        pos[0]         *= std::pow(0.025f, t) *
                         (length(pos[0]) > 10.0f ? 1.f : 0.f);
        pos[1]          = vec3();

        // Angular
        ang[0]         += t * ang[1];
        camera->yaw     = std::fmod(camera->yaw + t * ang[0].x,
                                    float(2 * M_PI));
        camera->pitch   = clamp(camera->pitch + t * ang[0].y,
                               -float(M_PI / 2 - 0.25f), -0.5f);
        ang[0]         *= std::pow(0.01f, t) *
                         (length(ang[0]) > 0.05f ? 1.f : 0.f);
        ang[1]          = vec3();

        return *this;
    }
};

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
    gfx::RenderStats stats;

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

        camera({0.f, 0.f, 0.f}, 350.f, M_PI / 2, -M_PI / 4,
               glm::radians(45.f), renderSize.aspect<float>(), 0.1, 750.f),

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
        SDL_PumpEvents();

        const float accPos = 1000.f, accAng = 10.f;
        const uint8_t* keyState = SDL_GetKeyboardState(nullptr);

        if (mouse.buttons()[0])
        {
            mouse.setCursor(platform::Mouse::Cursor::Hand);

            glm::vec3 w = camera.rayWorld(camera.rayEye(
                                          display->rayClip(mouse.position())));

            float d = 0;
            glm::intersectRayPlane(camera.position(), w,
                                   glm::vec3(), glm::vec3(0, 1, 0), d);

            glm::vec3 p  = camera.position() + d * w;
            glm::vec3 md = p - prevRayPos;
            md.y = 0;

            if (!glm::isNull(prevRayPos, 0.f))
                camera.target -= md;

            prevRayPos = camera.position() + d * w;
            //cameraControl.pos[1] = md * 100.f;
        }
        else
        {
            mouse.setCursor(platform::Mouse::Cursor::Arrow);
            prevRayPos = glm::vec3();
        }

        if (keyState[SDL_SCANCODE_LEFT]  || keyState[SDL_SCANCODE_A])
            cameraControl.pos[1].x = -accPos;
        if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D])
            cameraControl.pos[1].x = +accPos;
        if (keyState[SDL_SCANCODE_UP]    || keyState[SDL_SCANCODE_W])
            cameraControl.pos[1].y = +accPos;
        if (keyState[SDL_SCANCODE_DOWN]  || keyState[SDL_SCANCODE_S])
            cameraControl.pos[1].y = -accPos;

        if (keyState[SDL_SCANCODE_DELETE] || keyState[SDL_SCANCODE_Q])
            cameraControl.ang[1].x = -accAng;
        if (keyState[SDL_SCANCODE_END]    || keyState[SDL_SCANCODE_E])
            cameraControl.ang[1].x = +accAng;
        if (keyState[SDL_SCANCODE_PAGEUP])
            cameraControl.ang[1].y = 0.75f * -accAng;
        if (keyState[SDL_SCANCODE_PAGEDOWN])
            cameraControl.ang[1].y = 0.75f * +accAng;

        if (keyState[SDL_SCANCODE_ESCAPE])
            return false;

        cameraControl(&camera, step);
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
        grid(&lighting.fbo, proj * view * model);
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
        Scheduler scheduler(std::chrono::milliseconds(20),
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
