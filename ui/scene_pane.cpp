#include "scene_pane.h"

#include <include/screen.h>

#include "platform/display.h"

namespace pt
{
namespace ui
{

struct ScenePane::Data
{
    explicit Data(platform::Display* display) :
        display(display)
    {
        auto screen = display->nanoGui();
        screen->performLayout();
    }

    platform::Display* display;
};

ScenePane::ScenePane(platform::Display* display) :
    d(std::make_shared<Data>(display))
{}

} // namespace ui
} // namespace pt
