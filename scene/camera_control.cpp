#include "camera_control.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/vector_query.hpp>

#include "geom/ray.h"

namespace pt
{

struct CameraControl::Data
{
    Data(Camera* camera,
         platform::Display* display,
         platform::Mouse* mouse) :
        camera(camera),
        display(display),
        mouse(mouse)
    {}

    glm::vec3 mousePlanePos() const
    {
        const auto mousePos = display->clip(mouse->position());
        const auto rayWorld = camera->world(camera->eye(mousePos));
        float di = 0;
        glm::intersectRayPlane(rayWorld.pos, rayWorld.dir,
                               glm::vec3(), glm::vec3(0, 1, 0), di);
        return di * rayWorld.dir;
    }

    Camera*            camera;
    platform::Display* display;
    platform::Mouse*   mouse;

    // Velocity & acceleration vectors
    glm::vec3 pos[2], ang[2];
    glm::vec4 prevMousePos;
    glm::vec3 prevDragPos;
};

CameraControl::CameraControl(Camera* camera,
                             platform::Display* display,
                             platform::Mouse* mouse) :
    d(std::make_shared<Data>(camera, display, mouse))
{}

CameraControl& CameraControl::operator()(Duration step)
{
    using namespace glm;

    // Timestep
    const float t      = std::chrono::duration<float>(step).count();

    // Position
    d->pos[0]         += t * d->pos[1];
    d->camera->target += t * d->pos[0];
    d->pos[0]         *= std::pow(0.025f, t) *
                                 (length(d->pos[0]) > 5.0f ? 1.f : 0.f);
    d->pos[1]          = vec3();

    // Angular
    d->ang[0]         += t * d->ang[1];
    d->camera->yaw     = std::fmod(d->camera->yaw + t * d->ang[0].x,
                                   2 * glm::pi<float>());
    d->camera->pitch   = clamp(d->camera->pitch + t * d->ang[0].y,
                              -float(glm::half_pi<float>() - 0.25f), -0.5f);
    d->ang[0]         *= std::pow(0.01f, t) *
                                 (length(d->ang[0]) > 0.05f ? 1.f : 0.f);
    d->ang[1]          = vec3();

    // Process input
    const float accPos = 1000.f, accAng = 10.f;

    const glm::vec3 right   = d->camera->right();
    const glm::vec3 forward = normalize(glm::vec3(d->camera->forward().x,
                                                  0,
                                                  d->camera->forward().z));

    const uint8_t* keyState   = SDL_GetKeyboardState(nullptr);
    const glm::ivec2 mousePos = d->mouse->position();
    const bool mouseOnScene   = SDL_GetMouseFocus() &&
                                mousePos.x < d->display->size().w - 225;

    if (keyState[SDL_SCANCODE_LSHIFT] || keyState[SDL_SCANCODE_LCTRL])
        return *this;

    if (keyState[SDL_SCANCODE_LEFT]  || keyState[SDL_SCANCODE_A])
        d->pos[1] += -right * accPos;
    if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D])
        d->pos[1] +=  right * accPos;
    if (keyState[SDL_SCANCODE_UP]    || keyState[SDL_SCANCODE_W])
        d->pos[1] +=  forward * accPos;
    if (keyState[SDL_SCANCODE_DOWN]  || keyState[SDL_SCANCODE_S])
        d->pos[1] += -forward * accPos;

    if (keyState[SDL_SCANCODE_DELETE] || keyState[SDL_SCANCODE_Q])
        d->ang[1].x = +accAng;
    if (keyState[SDL_SCANCODE_END]    || keyState[SDL_SCANCODE_E])
        d->ang[1].x = -accAng;
    if (keyState[SDL_SCANCODE_PAGEUP] || (mouseOnScene && d->mouse->wheel() < 0))
        d->ang[1].y = 0.75f * -accAng * (1 + 2 * std::abs(d->mouse->wheel()));
    if (keyState[SDL_SCANCODE_PAGEDOWN] || (mouseOnScene && d->mouse->wheel() > 0))
        d->ang[1].y = 0.75f * +accAng * (1 + 2 * std::abs(d->mouse->wheel()));

    if (mouseOnScene)
    {
        const glm::vec4 rayMouse = d->display->clip(mousePos);
        const glm::vec3 rayDrag  = d->mousePlanePos();
        const glm::vec3 dragPos  = d->camera->position() + rayDrag;
        glm::vec3 md             = d->prevDragPos - dragPos;
        md.y                     = 0;

        const platform::Mouse::Buttons buttons = d->mouse->buttons();
        if (buttons[0] || buttons[2])
        {
            d->mouse->setCursor(platform::Mouse::Cursor::Hand);
            if (!glm::isNull(d->prevMousePos, 0.f))
            {
                if (buttons[0])
                {
                    d->camera->target += md;
                    d->ang[1] = glm::vec3();
                }
                else
                {
                    glm::vec4 mouseDiff = 32.f * (rayMouse - d->prevMousePos);
                    d->ang[1].x = mouseDiff.x * -accAng * 2;
                    d->ang[1].y = mouseDiff.y * +accAng * 2;
                }
            }
            d->prevMousePos = rayMouse;
            d->prevDragPos  = d->camera->position() + rayDrag;
        }
        else
        {
            d->mouse->setCursor(platform::Mouse::Cursor::Arrow);

            // Let position float after letting drag go
            if (!glm::isNull(d->prevMousePos, 0.f) &&
                 glm::isNull(d->ang[0], 0.f))
                d->pos[1] = 0.5f * glm::min(md, 10.f) * std::pow(1.f / t, 2.0f);

            d->prevMousePos = glm::vec4();
            d->prevDragPos  = glm::vec3();
        }
    }
    else
    {
        d->prevMousePos = glm::vec4();
        d->prevDragPos  = glm::vec3();
    }
    return *this;
}

} // namespace pt
