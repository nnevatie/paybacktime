#include "object_pane.h"

#include <glad/glad.h>
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

struct ObjectPane::Data
{
    explicit Data(platform::Display* display,
                  ObjectStore* objectStore,
                  TextureStore* textureStore) :
        display(display),
        objectStore(objectStore)
    {
        auto screen = display->nanoGui();

        auto& window = screen->add<nanogui::Window>("objects");
        window.setFixedSize({220, screen->size().y() - 16});
        window.setPosition({screen->size().x() - window.fixedWidth() - 8, 8});
        window.setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical));

        const Size<int> previewSize(256, 256);
        const Camera camera({0.f, 0.f, 0.f}, 90.f,
                             glm::quarter_pi<float>(), -glm::quarter_pi<float>(),
                             glm::radians(30.f),
                             previewSize.aspect<float>(), 0.1f, 100.f);

        gfx::Preview preview(previewSize);

        nanogui::ImagePanel::Images nvgImages;
        for (const auto& object : objectStore->objects())
        {
            const glm::vec3 dims = object.model().dimensions();
            const gl::Primitive primitive = object.model().primitive();

            const float s = std::pow(16.f / std::max(
                                            std::max(dims.x, dims.y), dims.z),
                                     0.4f);

            preview(&textureStore->albedo.texture, primitive,
                    camera.matrix() * glm::scale({}, glm::vec3(s, s, s)) *
                    glm::translate({}, glm::vec3(-dims.x, 4 - (1 - s) * 60.f, 0)));

            const Image image = preview.output()->image().flipped(Image::Axis::X);
            images.push_back(image);

            auto nvgImage = image.nvgImage(display->nanoVg());
            nvgImages.push_back({nvgImage, object.name()});
        }

        auto& vscroll = window.add<nanogui::VScrollPanel>();
        vscroll.setFixedSize({210, window.fixedHeight() - 36});

        auto& imagePanel = vscroll.add<nanogui::ImagePanel>(90, 5, 5);

        imagePanel.setCallback(
            [&imagePanel](int i)
        {
            imagePanel.setSelection(i);
        });

        imagePanel.setFixedSize({195, 512});
        imagePanel.setImages(nvgImages);
        imagePanel.setSelection(0);

        this->imagePanel = &imagePanel;
        screen->performLayout();
    }

    platform::Display*   display;
    ObjectStore*         objectStore;

    nanogui::ImagePanel* imagePanel;
    std::vector<Image>   images;
};

ObjectPane::ObjectPane(platform::Display* display,
                               ObjectStore* objectStore,
                               TextureStore* textureStore) :
    d(std::make_shared<Data>(display, objectStore, textureStore))
{
}

Object ObjectPane::selectedObject() const
{
    return d->objectStore->object(d->imagePanel->selection());
}

} // namespace ui
} // namespace pt
