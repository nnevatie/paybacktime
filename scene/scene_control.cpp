#include "scene_control.h"

#include <glm/gtc/matrix_transform.hpp>

#include "constants.h"

#include "common/log.h"

#include "geom/box.h"
#include "geom/ray.h"
#include "geom/transform.h"

#include "gfx/arrow.h"
#include "gfx/outline.h"

namespace pt
{

struct SceneControl::Data
{
    enum class State
    {
        Idle,
        Hovering,
        Adding,
        Removing
    };

    Data(Scene* scene,
         Camera* camera,
         platform::Display* display,
         platform::Mouse* mouse,
         const gl::Texture& texDepth) :
        scene(scene),
        camera(camera),
        display(display),
        mouse(mouse),
        outline(display->size(), texDepth),
        state(State::Idle)
    {}

    Scene*             scene;
    Camera*            camera;
    platform::Display* display;
    platform::Mouse*   mouse;

    gfx::Arrow         arrow;
    gfx::Outline       outline;

    State              state;

    Object             object;
    TransformTrRot     objectTransform;
};

SceneControl::SceneControl(Scene* scene,
                           Camera* camera,
                           platform::Display* display,
                           platform::Mouse* mouse,
                           const gl::Texture& texDepth) :
    d(std::make_shared<Data>(scene, camera, display, mouse, texDepth))
{}

SceneControl& SceneControl::operator()(Duration /*step*/, Object object)
{
    const auto mouseButtons = d->mouse->buttons();
    const auto mouseWheel   = d->mouse->wheel();
    const auto mousePos     = d->mouse->position();
    const bool mouseOnScene = mousePos.x < d->display->size().w - 225;

    if (mouseOnScene)
    {
        const uint8_t* keyState = SDL_GetKeyboardState(nullptr);
        if (keyState[SDL_SCANCODE_LSHIFT])
        {
            d->state = mouseButtons[0] ? Data::State::Adding   :
                       mouseButtons[2] ? Data::State::Removing :
                                         Data::State::Hovering;

            d->mouse->setCursor(platform::Mouse::Cursor::Crosshair);
        }
        else
            d->state = Data::State::Idle;

        if (d->state != Data::State::Idle)
        {
            // World intersection
            const auto clipRay  = d->display->clip(d->mouse->position());
            const auto rayWorld = d->camera->world(d->camera->eye(clipRay));
            auto intersection   = d->scene->intersect(rayWorld);

            // Translation
            auto& t = intersection.first;
            t      -= glm::mod(t, c::cell::GRID);
            t.y     = 0.f;
            t      += object.origin();

            // Rotation
            const auto dim = object.dimensions();
            const auto rot = umod(d->objectTransform.rot + mouseWheel, 4);
            t += glm::vec3(rot > 1            ? dim.x - c::cell::GRID.x : 0.f, 0.f,
                           rot > 0 && rot < 3 ? dim.z - c::cell::GRID.z : 0.f);

            if (d->state == Data::State::Adding &&
               !d->scene->contains({object, intersection.first}))
            {
                // Add item
                d->scene->add({object, {intersection.first,
                                        d->objectTransform.rot}});
            }
            else
            if (d->state == Data::State::Removing &&
                !intersection.second.empty())
            {
                // Remove items
                for (const auto& item : intersection.second)
                    d->scene->remove({item.obj, intersection.first});
            }
            // Store current state
            d->objectTransform.tr  = intersection.first;
            d->objectTransform.rot = rot;
        }
    }
    else
        d->state = Data::State::Idle;

    d->object = object;
    return *this;
}

SceneControl& SceneControl::operator()(gl::Fbo* fboOut, gl::Texture* texColor)
{
    if (const auto& object = d->object)
    {
        const auto m = static_cast<glm::mat4x4>(d->objectTransform);
        const auto v = d->camera->matrixView();
        const auto p = d->camera->matrixProj();

        if (d->state != Data::State::Idle)
        {
            const auto mvp = p * v * m;
            const auto t   = glm::vec3(0.f, d->object.dimensions().y, 0.f);
            d->outline(fboOut, texColor, object.model().primitive(), mvp);
            d->arrow(fboOut, mvp * glm::translate(t));
        }
    }
    return *this;
}

} // namespace pt
