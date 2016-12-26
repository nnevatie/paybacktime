#include "tools_window.h"

#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/widget.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/button.h>
#include <nanogui/stackedwidget.h>

#include "common/log.h"
#include "platform/display.h"

namespace pt
{
namespace ui
{
namespace ng = nanogui;

struct ToolsWindow::Data
{
    explicit Data(platform::Display* display)
    {
        auto screen  = display->nanoGui();
        auto& window = screen->add<ng::Window>(std::string());
        window.setFixedSize({220, screen->size().y() - 16});
        window.setPosition({screen->size().x() - window.fixedWidth() - 8, 8});
        window.setLayout(new nanogui::BoxLayout(ng::Orientation::Vertical,
                                                ng::Alignment::Fill, 5, 5));
        buttons = &window.add<ng::Widget>();
        stack   = &window.add<ng::StackedWidget>();
        buttons->setLayout(new nanogui::BoxLayout(ng::Orientation::Vertical,
                                                  ng::Alignment::Fill, 0, 5));
    }

    ng::Widget*        buttons;
    ng::StackedWidget* stack;
};

ToolsWindow::ToolsWindow(platform::Display* display) :
    d(std::make_shared<Data>(display))
{}

ToolsWindow& ToolsWindow::select(int index)
{
    d->stack->setSelectedIndex(index);
    return *this;
}

nanogui::Widget* ToolsWindow::add(const std::string& label, bool selected)
{
    const int index = d->buttons->childCount();
    auto stack      = d->stack;
    auto& button    = d->buttons->add<ng::Button>(label);
    button.setFlags(ng::Button::RadioButton);
    button.setPushed(selected);
    button.setChangeCallback([stack, index](bool state)
    {
        if (state) stack->setSelectedIndex(index);
    });
    return d->stack;
}

} // namespace ui
} // namespace pt
