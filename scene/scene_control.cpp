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
        auto dim = object.model().dimensions();
        auto c   = glm::vec3(0.5f * dim.x, 0.f, 0.5f * dim.z);
        auto m   = glm::translate({}, d->intersection.pos - c);
        auto v   = d->camera->matrixView();
        auto p   = d->camera->matrixProj();

        if (d->state == Data::State::Adding)
            d->outline(fboOut, texColor, object.model().primitive(), p * v * m);
    }
    return *this;
}

} // namespace pt
