#pragma once

#include "platform/clock.h"

namespace pt
{

struct SceneControl
{
    SceneControl& operator()(Duration step);
};

} // namespace pt
