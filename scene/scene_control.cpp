#include "scene_control.h"

#include <glm/gtc/matrix_transform.hpp>

#include "common/log.h"
#include "gfx/outline.h"

namespace pt
{

struct SceneControl::Data
{
    enum class State
    {
        Idle,
        Adding,
        Removing
    };

    Data(Scene* scene,
         Camera* camera,
         platform::Display* display,
         platform::Mouse* mouse,
         const gl::Texture& texDepth) :
        scene(scene),
        camera(camera),
        display(display),
        mouse(mouse),
        state(State::Idle),
        outline(display->size(), texDepth)
    {}

    Scene*             scene;
    Camera*            camera;
    platform::Display* display;
    platform::Mouse*   mouse;

    State              state;
    Intersection       intersection;
    Object             selectedObject;
    gfx::Outline       outline;
};

SceneControl::SceneControl(Scene* scene,
                           Camera* camera,
                           platform::Display* display,
                           platform::Mouse* mouse,
                           const gl::Texture& texDepth) :
    d(std::make_shared<Data>(scene, camera, display, mouse, texDepth))
{}

SceneControl& SceneControl::operator()(Duration /*step*/, Object selectedObject)
{
    const glm::ivec2 mousePos = d->mouse->position();
    const bool mouseOnScene   = mousePos.x < d->display->size().w - 225;

    if (mouseOnScene)
    {
        const uint8_t* keyState = SDL_GetKeyboardState(nullptr);

        if (keyState[SDL_SCANCODE_LSHIFT])
        {
            d->state = Data::State::Adding;
            d->mouse->setCursor(platform::Mouse::Cursor::Crosshair);
        }
        else
        if (keyState[SDL_SCANCODE_LCTRL])
        {
            d->state = Data::State::Removing;
            d->mouse->setCursor(platform::Mouse::Cursor::No);
        }
        else
            d->state = Data::State::Idle;

        const glm::vec4 clipRay  = d->display->rayClip(d->mouse->position());
        const glm::vec3 rayWorld = d->camera->rayWorld(d->camera->rayEye(clipRay));
        d-> intersection = d->scene->intersect(d->camera->position(), rayWorld);
    }
    else
        d->state = Data::State::Idle;

    d->selectedObject = selectedObject;
    return *this;
}

SceneControl& SceneControl::operator()(gl::Fbo* fboOut, gl::Texture* texColor)
{
    const Object object = d->selectedObject;
    if (object)
    {
        auto t   = d->intersection.pos;
        auto mx  = std::fmod(t.x, 16.f);
        auto mz  = std::fmod(t.z, 16.f);
        t.x     -= mx > 0 ? mx : (15.f + mx);
        t.z     -= mz > 0 ? mz : (15.f + mz);

        auto m   = glm::translate({}, t);
        auto v   = d->camera->matrixView();
        auto p   = d->camera->matrixProj();

        if (d->state == Data::State::Adding)
            d->outline(fboOut, texColor, object.model().primitive(), p * v * m);
    }
    return *this;
}

} // namespace pt
