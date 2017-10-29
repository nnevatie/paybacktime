#include "object_pane.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/button.h>
#include <nanogui/imageview.h>
#include <nanogui/imagepanel.h>
#include <nanogui/vscrollpanel.h>

#include "platform/display.h"
#include "gl/texture_atlas.h"
#include "geom/image_mesher.h"
#include "common/log.h"

#include "scene/camera.h"
#include "scene/object_store.h"
#include "scene/texture_store.h"

#include "gfx/preview.h"

namespace pt
{
namespace ui
{
namespace ng = nanogui;

struct ObjectPane::Data
{
    explicit Data(ng::Widget* parent,
                  platform::Display* display,
                  ObjectStore* objectStore,
                  TextureStore* textureStore) :
        objectStore(objectStore),
        widget(new ng::Widget(parent))
    {
        auto screen = display->nanoGui();
        widget->setFixedSize({220, screen->size().y() - 64});
        widget->setLayout(new ng::BoxLayout(ng::Orientation::Vertical));

        const Size<int> previewSize(256, 256);
        const Camera camera({0.f, 0.f, 0.f}, 90.f,
                             glm::quarter_pi<float>(), -glm::quarter_pi<float>(),
                             glm::radians(30.f),
                             previewSize.aspect<float>(), 0.1f, 200.f);

        gfx::Preview preview(previewSize);

        ng::ImagePanel::Images nvgImages;
        for (const auto& object : objectStore->objects())
            if (!object.parent())
            {
                preview(&textureStore->albedo.texture, camera, object);

                const Image image = preview.output()->image().
                                    flipped(Image::Axis::X);
                images.push_back(image);
                objectIds.push_back(object.id());

                auto nvgImage = image.nvgImage(display->nanoVg());
                nvgImages.push_back({nvgImage, object.name()});
            }

        auto& vscroll = widget->add<ng::VScrollPanel>();
        vscroll.setFixedSize({210, widget->fixedHeight() - 36});

        auto& imagePanel = vscroll.add<ng::ImagePanel>(90, 5, 5);
        imagePanel.setCallback([&imagePanel](int i)
        {
            imagePanel.setSelection(i);
        });

        imagePanel.setFixedSize({195, 512});
        imagePanel.setImages(nvgImages);
        imagePanel.setSelection(0);

        this->imagePanel = &imagePanel;
    }

    ObjectStore*       objectStore;
    ng::Widget*        widget;
    ng::ImagePanel*    imagePanel;
    std::vector<Image> images;
    Object::Ids        objectIds;
};

ObjectPane::ObjectPane(ng::Widget* parent,
                       platform::Display* display,
                       ObjectStore* objectStore,
                       TextureStore* textureStore) :
    d(std::make_shared<Data>(parent, display, objectStore, textureStore))
{
}

Object ObjectPane::selected() const
{
    return d->objectStore->object(d->objectIds.at(d->imagePanel->selection()));
}

} // namespace ui
} // namespace pt
