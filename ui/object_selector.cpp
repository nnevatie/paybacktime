#include "object_selector.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include <include/screen.h>
#include <include/window.h>
#include <include/layout.h>
#include <include/label.h>
#include <include/button.h>
#include <include/imageview.h>

#include "platform/display.h"
#include "geom/image_mesher.h"
#include "gfx/preview.h"
#include "gl/texture_atlas.h"
#include "scene/camera.h"

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

        const ImageCube depthCube("objects/box/*.png", 1);
        const ImageCube albedoCube("objects/box/albedo.*.png");

        gl::TextureAtlas atlas({256, 256}, 8);
        gl::TextureAtlas::EntryCube albedo = atlas.insert(albedoCube);

        const Mesh_P_N_UV mesh = ImageMesher::mesh(depthCube, albedo.second);
        const gl::Primitive primitive(mesh);

        const Size<int> previewSize(128, 128);

        Camera camera({0.f, 0.f, 0.f}, 100.f,
                      M_PI / 4, -M_PI / 3,
                      glm::radians(30.f),
                      previewSize.aspect<float>(), 0.01f, 250.f);

        gfx::Preview preview(previewSize);
        preview(&atlas.texture, primitive, camera.matrix() *
                                           glm::translate(glm::mat4x4(),
                                                          glm::vec3(-16, 0, 0)));

        image = preview.texColor.image().flipped();
        window.add<nanogui::ImageView>(image.nvgImage(display->nanoVg()));

        screen->performLayout();
    }

    ~Data()
    {
    }

    platform::Display* display;
    Image image;
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
