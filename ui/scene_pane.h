#pragma once

#include <memory>

namespace nanogui
{
class Widget;
}

namespace pt
{
namespace ui
{

struct ScenePane
{
    ScenePane(nanogui::Widget* parent);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
