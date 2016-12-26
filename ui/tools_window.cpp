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
    explicit Data(platform::Display* display, const Widgets& widgets)
    {
        auto screen  = display->nanoGui();
        auto& window = screen->add<ng::Window>(std::string());
        window.setFixedSize({220, screen->size().y() - 16});
        window.setPosition({screen->size().x() - window.fixedWidth() - 8, 8});
        window.setLayout(new nanogui::BoxLayout(ng::Orientation::Vertical,
                                                ng::Alignment::Fill, 5, 5));
        // Buttons
        std::vector<ng::Button*> buttons;
        for (int i = 0; i < int(widgets.size()); ++i)
        {
            const auto& widget = widgets.at(i);
            auto& button       = window.add<ng::Button>(widget.first);
            button.setFlags(ng::Button::RadioButton);
            button.setPushed(i == int(widgets.size() - 1));
            buttons.push_back(&button);
        }

        // Stack
        auto& stacked = window.add<ng::StackedWidget>();
        for (int i = 0; i < int(widgets.size()); ++i)
        {
            auto& widget = widgets.at(i);
            stacked.addChild(i, widget.second);
            buttons[i]->setChangeCallback([&stacked, i](bool state)
            {
                if (state) stacked.setSelectedIndex(i);
            });
        }
        screen->performLayout();
    }
};

ToolsWindow::ToolsWindow(platform::Display* display, const Widgets& widgets) :
    d(std::make_shared<Data>(display, widgets))
{}

} // namespace ui
} // namespace pt
