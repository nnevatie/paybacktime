#pragma once

#include <memory>
#include <vector>
#include <utility>

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

namespace ui
{

struct ToolsWindow
{
    using Widget  = std::pair<std::string, nanogui::Widget*>;
    using Widgets = std::vector<Widget>;

    ToolsWindow(platform::Display* display, const Widgets& widgets);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
