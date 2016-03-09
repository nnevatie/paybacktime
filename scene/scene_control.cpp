#include "scene_control.h"

#include <glm/gtc/matrix_transform.hpp>

#include "common/log.h"

#include "geom/box.h"
#include "geom/ray.h"
#include "geom/transform.h"

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
            const auto clipRay  = d->display->clip(d->mouse->position());
            const auto rayWorld = d->camera->world(d->camera->eye(clipRay));
            auto intersection   = d->scene->intersect(rayWorld);

            auto& t = intersection.first;
            auto mx = std::fmod(t.x, 16.f);
            auto mz = std::fmod(t.z, 16.f);
            t.x    -= mx > 0 ? mx : (16.f + mx);
            t.y     = 0;
            t.z    -= mz > 0 ? mz : (16.f + mz);
            t      += object.origin();

            if (d->state == Data::State::Adding &&
               !d->scene->contains({object, intersection.first}))
            {
                d->scene->add({object, {intersection.first,
                                        d->objectTransform.rot}});
            }
            else
            if (d->state == Data::State::Removing &&
                !intersection.second.empty())
            {
                for (const auto& item : intersection.second)
                    d->scene->remove({item.obj, intersection.first});
            }
            d->objectTransform.tr  = intersection.first;
            d->objectTransform.rot = umod(d->objectTransform.rot + mouseWheel, 4);
        }
    }
    else
        d->state = Data::State::Idle;

    d->object = object;
    return *this;
}

SceneControl& SceneControl::operator()(gl::Fbo* fboOut, gl::Texture* texColor)
{
    if (const Object object = d->object)
    {
        auto m = static_cast<glm::mat4x4>(d->objectTransform);
        auto v = d->camera->matrixView();
        auto p = d->camera->matrixProj();

        if (d->state != Data::State::Idle)
            d->outline(fboOut, texColor, object.model().primitive(), p * v * m);
    }
    return *this;
}

} // namespace pt
