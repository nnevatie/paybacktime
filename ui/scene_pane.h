#pragma once

#include <memory>

namespace nanogui
{
class Widget;
}

namespace pt
{
struct Scene;
struct HorizonStore;

namespace ui
{

struct ScenePane
{
    ScenePane(nanogui::Widget* parent,
              Scene* scene, HorizonStore* horizonStore);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
