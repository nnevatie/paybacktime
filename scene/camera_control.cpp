#include "camera_control.h"

#include <glm/gtx/intersect.hpp>
#include <glm/gtx/vector_query.hpp>

namespace pt
{

CameraControl::CameraControl(Camera* camera,
                             platform::Display* display,
                             platform::Mouse* mouse) :
    camera(camera),
    display(display),
    mouse(mouse)
{}

glm::vec3 CameraControl::mousePlanePos() const
{
    const glm::vec4 mousePos = display->rayClip(mouse->position());
    const glm::vec3 rayWorld = camera->rayWorld(camera->rayEye(mousePos));
    float di = 0;
    glm::intersectRayPlane(camera->position(), rayWorld,
                           glm::vec3(), glm::vec3(0, 1, 0), di);
    return di * rayWorld;
}

CameraControl& CameraControl::operator()(Duration step)
{
    using namespace glm;

    // Timestep
    const float t   = std::chrono::duration<float>(step).count();

    // Position
    pos[0]         += t * pos[1];
    camera->target += t * pos[0];
    pos[0]         *= std::pow(0.025f, t) *
                     (length(pos[0]) > 5.0f ? 1.f : 0.f);
    pos[1]          = vec3();

    // Angular
    ang[0]         += t * ang[1];
    camera->yaw     = std::fmod(camera->yaw + t * ang[0].x,
                                float(2 * M_PI));
    camera->pitch   = clamp(camera->pitch + t * ang[0].y,
                           -float(M_PI / 2 - 0.25f), -0.5f);
    ang[0]         *= std::pow(0.01f, t) *
                     (length(ang[0]) > 0.05f ? 1.f : 0.f);
    ang[1]          = vec3();

    // Process input
    const float accPos = 1000.f, accAng = 10.f;

    const glm::vec3 right   = camera->right();
    const glm::vec3 forward = normalize(glm::vec3(camera->forward().x,
                                                  0,
                                                  camera->forward().z));

    const uint8_t* keyState   = SDL_GetKeyboardState(nullptr);
    const glm::ivec2 mousePos = mouse->position();
    const bool mouseOnScene   = mousePos.x < display->size().w - 225;

    if (keyState[SDL_SCANCODE_LEFT]  || keyState[SDL_SCANCODE_A])
        pos[1] += -right * accPos;
    if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D])
        pos[1] +=  right * accPos;
    if (keyState[SDL_SCANCODE_UP]    || keyState[SDL_SCANCODE_W])
        pos[1] +=  forward * accPos;
    if (keyState[SDL_SCANCODE_DOWN]  || keyState[SDL_SCANCODE_S])
        pos[1] += -forward * accPos;

    if (keyState[SDL_SCANCODE_DELETE] || keyState[SDL_SCANCODE_Q])
        ang[1].x = +accAng;
    if (keyState[SDL_SCANCODE_END]    || keyState[SDL_SCANCODE_E])
        ang[1].x = -accAng;
    if (keyState[SDL_SCANCODE_PAGEUP] || (mouseOnScene && mouse->wheel() < 0))
        ang[1].y = 0.75f * -accAng * (1 + 2 * std::abs(mouse->wheel()));
    if (keyState[SDL_SCANCODE_PAGEDOWN] || (mouseOnScene && mouse->wheel() > 0))
        ang[1].y = 0.75f * +accAng * (1 + 2 * std::abs(mouse->wheel()));

    if (mouseOnScene)
    {
        const glm::vec4 rayMouse = display->rayClip(mousePos);
        const glm::vec3 rayDrag  = mousePlanePos();
        const glm::vec3 dragPos  = camera->position() + rayDrag;
        glm::vec3 md             = prevDragPos - dragPos;
        md.y                     = 0;

        const platform::Mouse::Buttons buttons = mouse->buttons();
        if (buttons[0] || buttons[2])
        {
            mouse->setCursor(platform::Mouse::Cursor::Hand);
            if (!glm::isNull(prevMousePos, 0.f))
            {
                if (buttons[0])
                {
                    camera->target += md;
                    ang[1] = glm::vec3();
                }
                else
                {
                    glm::vec4 mouseDiff = 32.f * (rayMouse - prevMousePos);
                    ang[1].x = mouseDiff.x * -accAng * 2;
                    ang[1].y = mouseDiff.y * +accAng * 2;
                }
            }
            prevMousePos = rayMouse;
            prevDragPos  = camera->position() + rayDrag;
        }
        else
        {
            mouse->setCursor(platform::Mouse::Cursor::Arrow);

            // Let position float after letting drag go
            if (!glm::isNull(prevMousePos, 0.f) &&
                    glm::isNull(ang[0], 0.f))
                pos[1] = 0.5f * md * std::pow(1.f / t, 2.0f);

            prevMousePos = glm::vec4();
            prevDragPos  = glm::vec3();
        }
    }
    else
    {
        prevMousePos = glm::vec4();
        prevDragPos  = glm::vec3();
    }
    return *this;
}

} // namespace pt
