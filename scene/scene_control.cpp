#include "scene_control.h"

namespace pt
{

SceneControl::SceneControl(Camera* camera,
                           platform::Display* display,
                           platform::Mouse* mouse) :
    camera(camera),
    display(display),
    mouse(mouse)
{}

SceneControl& SceneControl::operator()(Duration /*step*/)
{
    const uint8_t* keyState = SDL_GetKeyboardState(nullptr);

    if (keyState[SDL_SCANCODE_LSHIFT])
        mouse->setCursor(platform::Mouse::Cursor::Crosshair);
    else
    if (keyState[SDL_SCANCODE_LCTRL])
        mouse->setCursor(platform::Mouse::Cursor::No);

    return *this;
}

} // namespace pt
