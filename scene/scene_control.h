#pragma once

#include <memory>

#include "platform/clock.h"
#include "platform/display.h"
#include "platform/mouse.h"

#include "gl/fbo.h"

#include "camera.h"
#include "scene.h"

namespace pt
{

struct SceneControl
{
    SceneControl(Scene* scene,
                 Camera* camera,
                 platform::Display* display,
                 platform::Mouse* mouse);

    SceneControl& operator()(Duration step, Object selectedObject);
    SceneControl& operator()(gl::Fbo* fboOut);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
