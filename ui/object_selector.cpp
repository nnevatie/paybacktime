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
#include <include/imagepanel.h>

#include "gl/texture_atlas.h"
#include "platform/display.h"
#include "geom/image_mesher.h"
#include "scene/camera.h"

#include "gfx/preview.h"
#include "gfx/anti_alias.h"

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

        const Size<int> previewSize(256, 256);

        Camera camera({0.f, 0.f, 0.f}, 90.f,
                      M_PI / 4, -M_PI / 4,
                      glm::radians(30.f),
                      previewSize.aspect<float>(), 0.1f, 100.f);

        gfx::Preview preview(previewSize);
        preview(&atlas.texture, primitive, camera.matrix() *
                                           glm::translate(glm::mat4x4(),
                                                          glm::vec3(-16, 4, 0)));
        gfx::AntiAlias aa(previewSize);
        aa(&preview.texDenoise);

        image = aa.output()->image().flipped();
        auto& img = window.add<nanogui::ImagePanel>(90, 5, 5);
        img.setFixedSize({195, 512});

        auto nvgImage = image.nvgImage(display->nanoVg());
        nanogui::ImagePanel::Images images;
        images.push_back({nvgImage, "box1"});
        images.push_back({nvgImage, "box2"});
        images.push_back({nvgImage, "box3"});
        images.push_back({nvgImage, "box4"});
        img.setImages(images);

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
