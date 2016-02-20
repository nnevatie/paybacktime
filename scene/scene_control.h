#pragma once

#include "scene/camera.h"

#include "platform/clock.h"
#include "platform/display.h"
#include "platform/mouse.h"

namespace pt
{

struct SceneControl
{
    Camera*            camera;
    platform::Display* display;
    platform::Mouse*   mouse;

    SceneControl(Camera* camera,
                 platform::Display* display,
                 platform::Mouse* mouse);

    SceneControl& operator()(Duration step);
};

} // namespace pt
