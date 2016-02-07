#include "scene_control.h"

namespace pt
{

SceneControl& SceneControl::operator()(Duration step)
{
    return *this;
}

} // namespace pt
