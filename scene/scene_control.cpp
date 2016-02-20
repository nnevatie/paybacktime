#include "scene_control.h"

namespace pt
{

struct SceneControl::Data
{
    Data(Scene* scene,
         Camera* camera,
         platform::Display* display,
         platform::Mouse* mouse) :
        scene(scene),
        camera(camera),
        display(display),
        mouse(mouse)
    {}

    Scene*             scene;
    Camera*            camera;
    platform::Display* display;
    platform::Mouse*   mouse;
};

SceneControl::SceneControl(Scene* scene,
                           Camera* camera,
                           platform::Display* display,
                           platform::Mouse* mouse) :
    d(std::make_shared<Data>(scene, camera, display, mouse))
{}

SceneControl& SceneControl::operator()(Duration /*step*/, Object selectedObject)
{
    const uint8_t* keyState = SDL_GetKeyboardState(nullptr);

    if (keyState[SDL_SCANCODE_LSHIFT])
        d->mouse->setCursor(platform::Mouse::Cursor::Crosshair);
    else
    if (keyState[SDL_SCANCODE_LCTRL])
        d->mouse->setCursor(platform::Mouse::Cursor::No);

    const glm::ivec2 mousePos = d->mouse->position();
    const bool mouseOnScene   = mousePos.x < d->display->size().w - 225;

    if (mouseOnScene)
    {
        const glm::vec4 clipRay  = d->display->rayClip(d->mouse->position());
        const glm::vec3 rayWorld = d->camera->rayWorld(d->camera->rayEye(clipRay));

        const Intersection intersection =
            d->scene->intersect(d->camera->position(), rayWorld);
    }
    return *this;
}

SceneControl& SceneControl::operator()(gl::Fbo* fboOut)
{
    return *this;
}

} // namespace pt
