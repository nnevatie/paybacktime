#include "scene_pane.h"

#include <nanogui/widget.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/button.h>
#include <nanogui/combobox.h>

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
        widget->setLayout(new ng::BoxLayout(ng::Orientation::Vertical,
                                            ng::Alignment::Fill, 5, 5));
        widget->add<ng::Label>("Horizon");

        std::vector<std::string> horizons = {"None", "80s", "Test"};
        widget->add<ng::ComboBox>(horizons);

        widget->setVisible(false);
    }

    ng::Widget* widget;
};

ScenePane::ScenePane(ng::Widget* parent) :
    d(std::make_shared<Data>(parent))
{}

} // namespace ui
} // namespace pt
