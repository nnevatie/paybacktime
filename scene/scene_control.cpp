#include "scene_control.h"

namespace pt
{

SceneControl::SceneControl(Scene* scene,
                           Camera* camera,
                           platform::Display* display,
                           platform::Mouse* mouse) :
    scene(scene),
    camera(camera),
    display(display),
    mouse(mouse)
{}

SceneControl& SceneControl::operator()(Duration /*step*/, Object selectedObject)
{
    const uint8_t* keyState = SDL_GetKeyboardState(nullptr);

    if (keyState[SDL_SCANCODE_LSHIFT])
        mouse->setCursor(platform::Mouse::Cursor::Crosshair);
    else
    if (keyState[SDL_SCANCODE_LCTRL])
        mouse->setCursor(platform::Mouse::Cursor::No);

    const glm::ivec2 mousePos = mouse->position();
    const bool mouseOnScene   = mousePos.x < display->size().w - 225;

    if (mouseOnScene)
    {
        const glm::vec4 clipRay  = display->rayClip(mouse->position());
        const glm::vec3 rayWorld = camera->rayWorld(camera->rayEye(clipRay));

        const Intersection intersection =
            scene->intersect(camera->position(), rayWorld);
    }
    return *this;
}

SceneControl& SceneControl::operator()(gl::Fbo* fboOut)
{
    return *this;
}

} // namespace pt
