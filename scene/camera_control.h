#pragma once

#include "scene/camera.h"

#include "platform/clock.h"
#include "platform/display.h"
#include "platform/mouse.h"

namespace pt
{

struct CameraControl
{
    Camera*            camera;
    platform::Display* display;
    platform::Mouse*   mouse;

    // Velocity & acceleration vectors
    glm::vec3 pos[2], ang[2];
    glm::vec4 prevMousePos;
    glm::vec3 prevDragPos;

    CameraControl(Camera* camera,
                  platform::Display* display,
                  platform::Mouse* mouse);

    glm::vec3 mousePlanePos() const;

    CameraControl& operator()(Duration step);
};

} // namespace pt
