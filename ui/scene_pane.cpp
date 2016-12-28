#include "scene_pane.h"

#include <nanogui/widget.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/button.h>
#include <nanogui/combobox.h>

#include "scene/scene.h"
#include "scene/horizon_store.h"

namespace pt
{
namespace ui
{
namespace ng = nanogui;

struct ScenePane::Data
{
    explicit Data(ng::Widget* parent, Scene* scene, HorizonStore* horizonStore) :
        horizonStore(horizonStore),
        widget(new ng::Widget(parent))
    {
        widget->setLayout(new ng::BoxLayout(ng::Orientation::Vertical,
                                            ng::Alignment::Fill, 5, 5));
        // Horizons
        widget->add<ng::Label>("Horizon");
        const auto horizons = horizonStore->horizons();
        std::vector<std::string> horizonNames;
        std::transform(horizons.begin(), horizons.end(),
                       std::back_inserter(horizonNames),
                       [](Horizon h) {return h.name();});

        horizonSelector = &widget->add<ng::ComboBox>(horizonNames);
        horizonSelector->setCallback([scene, horizonStore](int index)
        {
            scene->setHorizon(horizonStore->horizon(index));
        });
        widget->setVisible(false);
    }

    HorizonStore* horizonStore;
    ng::Widget*   widget;
    ng::ComboBox* horizonSelector;
};

ScenePane::ScenePane(ng::Widget* parent,
                     Scene* scene, HorizonStore* horizonStore) :
    d(std::make_shared<Data>(parent, scene, horizonStore))
{}

} // namespace ui
} // namespace pt
