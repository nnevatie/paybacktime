#pragma once

#include <memory>

#include "scene/camera.h"

#include "platform/clock.h"
#include "platform/display.h"
#include "platform/mouse.h"

namespace pt
{

struct CameraControl
{
    CameraControl(Camera* camera,
                  platform::Display* display,
                  platform::Mouse* mouse);

    CameraControl& operator()(Duration step);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
