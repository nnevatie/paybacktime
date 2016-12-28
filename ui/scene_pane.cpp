#include "scene_pane.h"

#include <nanogui/widget.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/button.h>
#include <nanogui/combobox.h>
#include <nanogui/imageview.h>

#include "platform/display.h"

#include "scene/scene.h"
#include "scene/horizon_store.h"

namespace pt
{
namespace ui
{
namespace ng = nanogui;

struct ScenePane::Data
{
    explicit Data(ng::Widget* parent,
                  platform::Display* display,
                  Scene* scene,
                  HorizonStore* horizonStore) :
        widget(new ng::Widget(parent))
    {
        auto nanovg = display->nanoVg();

        widget->setLayout(new ng::BoxLayout(ng::Orientation::Vertical,
                                            ng::Alignment::Fill, 5, 5));
        // Horizons
        widget->add<ng::Label>("Horizon");
        const auto horizons = horizonStore->horizons();
        std::vector<std::string> horizonNames;
        std::transform(horizons.begin(), horizons.end(),
                       std::back_inserter(horizonNames),
                       [](Horizon h) {return h.name();});

        auto horizonView     = &widget->add<ng::ImageView>(
                               horizons.front().preview().nvgImage(nanovg));
        auto horizonSelector = &widget->add<ng::ComboBox>(horizonNames);

        horizonSelector->setCallback(
            [scene, horizonStore, horizonView, nanovg](int index)
            {
                const auto horizon = horizonStore->horizon(index);
                scene->setHorizon(horizon);
                horizonView->setImage(horizon.preview().nvgImage(nanovg));
            });

        widget->setVisible(false);
    }

    ng::Widget* widget;
};

ScenePane::ScenePane(ng::Widget* parent,
                     platform::Display* display,
                     Scene* scene,
                     HorizonStore* horizonStore) :
    d(std::make_shared<Data>(parent, display, scene, horizonStore))
{}

} // namespace ui
} // namespace pt
