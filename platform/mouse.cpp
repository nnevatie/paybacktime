#include "mouse.h"

#include <vector>

#include <SDL.h>

#include "common/log.h"

namespace pt
{
namespace platform
{
using Cursors = std::vector<SDL_Cursor*>;

struct Mouse::Data
{
    Cursors cursors;
    Buttons buttons;
    int     wheel;

    Data() :
        buttons({}),
        wheel(0)
    {
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
    d->wheel = {};
    return *this;
}

Mouse& Mouse::update(const SDL_Event& event)
{
    if (event.type == SDL_MOUSEWHEEL)
        d->wheel += event.wheel.y;

    // Clear button states on window defocus
    if (event.type         == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_LEAVE)
        d->buttons = {};

    if (event.type == SDL_MOUSEBUTTONDOWN ||
        event.type == SDL_MOUSEBUTTONUP)
    {
        const int index   = event.button.button - SDL_BUTTON_LEFT;
        d->buttons[index] = event.type == SDL_MOUSEBUTTONDOWN ? 1 : 0;
    }
    return *this;
}

} // namespace platform
} // namespace pt
