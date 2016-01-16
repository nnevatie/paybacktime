#include "object_selector.h"

#include <glad/glad.h>

#include <include/screen.h>
#include <include/window.h>
#include <include/layout.h>
#include <include/label.h>
#include <include/button.h>

#include "platform/display.h"

namespace pt
{
namespace ui
{

struct ObjectSelector::Data
{
    explicit Data(platform::Display* display) :
        display(display)
    {
        nanogui::Screen* screen = display->nanoGui();
        auto& window = screen->add<nanogui::Window>("Objects");
        window.setFixedSize({200, screen->size().y() - 16});
        window.setPosition({screen->size().x() - window.fixedSize().x() - 8, 8});
        window.setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical));
        window.add<nanogui::Label>("Label", "sans-bold");
        screen->performLayout();
    }

    ~Data()
    {
    }

    platform::Display* display;
};

ObjectSelector::ObjectSelector(platform::Display* display) :
    d(new Data(display))
{
}

ObjectSelector& ObjectSelector::operator()()
{
    d->display->nanoGui()->drawAll();
    return *this;
}

} // namespace ui
} // namespace pt
