#include "scene_pane.h"

#include <queue>

#include <nanogui/widget.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/button.h>
#include <nanogui/combobox.h>
#include <nanogui/imageview.h>

#include "platform/display.h"

#include "scene/scene.h"
#include "scene/object_store.h"
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
                  HorizonStore* horizonStore,
                  ObjectStore* objectStore) :
        widget(new ng::Widget(parent))
    {
        auto nanovg = display->nanoVg();

        // Layout
        widget->setLayout(new ng::BoxLayout(ng::Orientation::Vertical,
                                            ng::Alignment::Fill, 5, 5));

        // Persistence
        const auto fileType = std::make_pair<std::string, std::string>
                             ("scn", "Payback Time Scene");

        widget->add<ng::Label>("File");
        widget->add<ng::Button>("Load").setCallback([=]
        {
            actions.push([=]()
            {
                std::string path;
                {
                    PathPreserver pathPreserver;
                    path = ng::file_dialog({fileType}, false);
                }
                if (!path.empty())
                    *scene = Scene(path, *horizonStore, *objectStore);
            });
        });
        widget->add<ng::Button>("Save").setCallback([=]
        {
            actions.push([=]()
            {
                PathPreserver pathPreserver;
                const auto path = ng::file_dialog({fileType}, true);
                if (!path.empty())
                    scene->write(path);
            });
        });

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

        // Set current item
        if (const auto horizon = horizonStore->horizon(scene->horizon().name()))
        {
            const auto it = std::find(horizons.begin(), horizons.end(), horizon);
            horizonView->setImage(horizon.preview().nvgImage(nanovg));

            horizonSelector->setSelectedIndex(
                int(std::distance(horizons.begin(), it)));
        }

        // Set selection callback
        horizonSelector->setCallback(
            [scene, horizonStore, horizonView, nanovg](int index)
            {
                const auto horizon = horizonStore->horizon(index);
                scene->setHorizon(horizon);
                horizonView->setImage(horizon.preview().nvgImage(nanovg));
            });

        widget->setVisible(false);
    }

    ng::Widget*                   widget;
    std::queue<ScenePane::Action> actions;
};

ScenePane::ScenePane(ng::Widget* parent,
                     platform::Display* display,
                     Scene* scene,
                     HorizonStore* horizonStore,
                     ObjectStore* objectStore) :
    d(std::make_shared<Data>(parent, display, scene, horizonStore, objectStore))
{}

ScenePane::Action ScenePane::nextAction()
{
    if (!d->actions.empty())
    {
        const Action action = d->actions.front();
        d->actions.pop();
        return action;
    }
    return {};
}

} // namespace ui
} // namespace pt
