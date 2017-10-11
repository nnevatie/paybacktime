#include "scene_control.h"

#include <glm/gtc/matrix_transform.hpp>

#include "constants.h"

#include "common/log.h"

#include "geom/aabb.h"
#include "geom/ray.h"
#include "geom/transform.h"

#include "gfx/arrow.h"
#include "gfx/outline.h"

namespace pt
{
using OutlineTr = std::pair<glm::mat4x4, glm::mat4x4>;

namespace
{

OutlineTr outlineTransform(Camera* camera, const ObjectItem& object)
{
    const auto m   = object.xform.matrix(object.obj.dimensions());
    const auto v   = camera->matrixView();
    const auto p   = camera->matrixProj();
    const auto mvp = p * v * m;
    const auto t   = glm::vec3(0.f, object.obj.dimensions().y, 0.f);
    return OutlineTr(mvp, mvp * glm::translate(t));
}

} // namespace

struct SceneControl::Data
{
    enum class State
    {
        Idle,
        Adding,
        Removing
    };

    Data(Scene* scene,
         Camera* camera,
         platform::Display* display,
         platform::Mouse* mouse,
         const Size<int>& renderSize,
         const gl::Texture& texDepth) :
        scene(scene),
        camera(camera),
        display(display),
        mouse(mouse),
        outline(renderSize, texDepth),
        state(State::Idle)
    {}

    Scene*             scene;
    Camera*            camera;
    platform::Display* display;
    platform::Mouse*   mouse;

    gfx::Arrow         arrow;
    gfx::Outline       outline;

    State              state;

    ObjectItem         object;
    Intersection       intersection;
    Object             removedObject;
};

SceneControl::SceneControl(Scene* scene,
                           Camera* camera,
                           platform::Display* display,
                           platform::Mouse* mouse,
                           const Size<int>& renderSize,
                           const gl::Texture& texDepth) :
    d(std::make_shared<Data>(scene, camera, display, mouse, renderSize, texDepth))
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

        d->state = keyState[SDL_SCANCODE_LSHIFT] ? Data::State::Adding :
                   keyState[SDL_SCANCODE_LCTRL]  ? Data::State::Removing :
                                                   Data::State::Idle;
        if (d->state != Data::State::Idle)
        {
            d->mouse->setCursor(platform::Mouse::Cursor::Crosshair);

            // World intersection
            const auto clipRay  = d->display->clip(d->mouse->position());
            const auto rayWorld = d->camera->world(d->camera->eye(clipRay));
            auto intersection   = d->scene->intersect(rayWorld);

            // Store intersection
            d->intersection = intersection;

            // Rotation
            d->object.xform.rot = umod(d->object.xform.rot + mouseWheel,
                                       c::scene::ROT_TICKS);
            // Translation
            const auto aabb = d->object.bounds();
            const auto div  = aabb.size() - glm::mod(aabb.size(), c::cell::GRID);
            const auto dim  = glm::any(glm::equal(div, glm::zero<glm::vec3>())) ?
                              aabb.size() : div;
            const auto grid = glm::min(dim, c::cell::GRID);

            auto& t = intersection.first;
            auto  r = glm::mod(t, grid) - glm::mod(0.5f * dim, grid);
            t       = glm::round(t - r);
            t.y     = 0.f;
            t      += object.origin();

            d->object.xform.pos = t;

            const ObjectItem objectItem(object, d->object.xform);

            if (d->state == Data::State::Adding)
            {
                // Add item
                if (mouseButtons[0] && d->scene->intersect(objectItem, 0.1f).empty())
                    d->scene->add(objectItem);
                else
                if (mouseButtons[2] && !d->scene->contains(objectItem.bounds()))
                    d->scene->add(objectItem);
            }
            else
            if (d->state == Data::State::Removing && mouseButtons[0] &&
               !d->intersection.second.empty())
            {
                // Remove items
                const auto firstObj = d->intersection.second.front();
                if (!d->removedObject || firstObj.obj == d->removedObject)
                {
                    d->removedObject = firstObj.obj;
                    d->scene->remove(firstObj);
                }
            }
            else
            if (!mouseButtons[0])
                // Reset removed object type
                d->removedObject = {};
        }
    }
    else
        d->state = Data::State::Idle;

    d->object.obj = object;
    return *this;
}

SceneControl& SceneControl::operator()(gl::Fbo* fboOut, gl::Texture* texColor)
{
    if (d->state == Data::State::Removing)
        if (!d->intersection.second.empty())
        {
            // Prefer same-typed removed objects
            ObjectItem firstObj;
            if (!d->removedObject)
                firstObj = d->intersection.second.front();
            else
                for (const auto intObj : d->intersection.second)
                    if (intObj.obj == d->removedObject)
                    {
                        firstObj = intObj;
                        break;
                    }

            if (firstObj.obj)
            {
                const auto otr = outlineTransform(d->camera, firstObj);
                d->outline(fboOut, texColor,
                           firstObj.obj.model().primitive(), otr.first,
                           glm::vec4(0.75f, 0.f, 0.f, 1.f));
            }
        }

    if (d->state == Data::State::Adding)
        if (const auto& object = d->object)
        {
            const auto otr = outlineTransform(d->camera, d->object);
            d->outline(fboOut, texColor,
                       object.obj.model().primitive(), otr.first,
                       glm::vec4(0.f, 0.5f, 0.75f, 1.f));
            d->arrow(fboOut, otr.second);
        }
    return *this;
}

} // namespace pt
