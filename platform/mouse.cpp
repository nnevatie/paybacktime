#include "mouse.h"

#include <vector>

#include <SDL2/SDL.h>

#include "common/log.h"

namespace pt
{
namespace platform
{

struct Mouse::Data
{
    std::vector<SDL_Cursor*> cursors;
    int                      wheel;
    Buttons                  buttons;
    Buttons                  transitions;

    Data() :
        wheel(0)
    {
        buttons.fill(0);
        transitions.fill(0);

        SDL_SystemCursor cursorIds[] = {
            SDL_SYSTEM_CURSOR_ARROW,
            SDL_SYSTEM_CURSOR_WAITARROW,
            SDL_SYSTEM_CURSOR_CROSSHAIR,
            SDL_SYSTEM_CURSOR_NO,
            SDL_SYSTEM_CURSOR_HAND,
            SDL_SYSTEM_CURSOR_SIZEALL
        };
        for (auto cursorId : cursorIds)
            cursors.push_back(SDL_CreateSystemCursor(cursorId));
    }
    ~Data()
    {
        for (auto cursor : cursors)
            SDL_FreeCursor(cursor);
    }
};

Mouse::Mouse() :
    d(std::make_shared<Data>())
{
}

glm::ivec2 Mouse::position() const
{
    glm::ivec2 pos;
    SDL_GetMouseState(&pos.x, &pos.y);
    return pos;
}

Mouse::Buttons Mouse::buttons() const
{
    return d->buttons;
}

Mouse::Buttons Mouse::buttonTransitions() const
{
    return d->transitions;
}

int Mouse::wheel() const
{
    return d->wheel;
}

Mouse& Mouse::setCursor(Mouse::Cursor cursor)
{
    const int index = int(cursor);
    if (index >= 0 && index < int(d->cursors.size()))
        SDL_SetCursor(d->cursors.at(index));

    return *this;
}

Mouse& Mouse::reset()
{
    d->wheel       = {};
    d->transitions = {};
    return *this;
}

Mouse& Mouse::update(const SDL_Event& event)
{
    if (event.type == SDL_MOUSEWHEEL)
        d->wheel += event.wheel.y;

    // Clear button states on window defocus
    if (event.type         == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_LEAVE)
        d->buttons.fill(0);

    if (event.type == SDL_MOUSEBUTTONDOWN ||
        event.type == SDL_MOUSEBUTTONUP)
    {
        const int index = event.button.button == SDL_BUTTON_LEFT   ? 0 :
                          event.button.button == SDL_BUTTON_MIDDLE ? 1 : 2;

        d->buttons[index]     = event.type == SDL_MOUSEBUTTONDOWN ? 1 :  0;
        d->transitions[index] = event.type == SDL_MOUSEBUTTONDOWN ? 1 : -1;
    }
    return *this;
}

} // namespace platform
} // namespace pt
