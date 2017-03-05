#pragma once

#include <memory>

namespace nanogui
{
class Widget;
}

namespace pt
{
namespace platform
{
struct Display;
}

struct Scene;
struct ObjectStore;
struct HorizonStore;

namespace ui
{

struct ScenePane
{
    ScenePane(nanogui::Widget* parent,
              platform::Display* display,
              Scene* scene,
              HorizonStore* horizonStore,
              ObjectStore* objectStore);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
