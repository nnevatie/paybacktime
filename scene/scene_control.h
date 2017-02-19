#pragma once

#include <memory>

#include "platform/clock.h"
#include "platform/display.h"
#include "platform/mouse.h"

#include "gl/texture.h"
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
                 platform::Mouse* mouse,
                 const Size<int>& renderSize,
                 const gl::Texture& texDepth);

    SceneControl& operator()(Duration step, Object object);
    SceneControl& operator()(gl::Fbo* fboOut,
                             gl::Texture* texColor);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
