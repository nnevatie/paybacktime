#include "scene_pane.h"

#include <nanogui/widget.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/button.h>

namespace pt
{
namespace ui
{
namespace ng = nanogui;

struct ScenePane::Data
{
    explicit Data(ng::Widget* parent) :
        widget(new ng::Widget(parent))
    {
    }

    ng::Widget* widget;
};

ScenePane::ScenePane(ng::Widget* parent) :
    d(std::make_shared<Data>(parent))
{}

} // namespace ui
} // namespace pt
