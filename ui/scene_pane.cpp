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
    explicit Data() :
        widget(nullptr)
    {
    }

    ng::Widget widget;
};

ScenePane::ScenePane() :
    d(std::make_shared<Data>())
{}

nanogui::Widget* ScenePane::widget() const
{
    return &d->widget;
}

} // namespace ui
} // namespace pt
