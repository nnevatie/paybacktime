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

    Data()
    {
        SDL_SystemCursor cursorIds[] = {
            SDL_SYSTEM_CURSOR_ARROW,
            SDL_SYSTEM_CURSOR_WAITARROW,
            SDL_SYSTEM_CURSOR_CROSSHAIR,
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
    d(new Data())
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
    const uint32_t mask = SDL_GetMouseState(nullptr, nullptr);
    return {(mask & SDL_BUTTON_LMASK) > 0,
            (mask & SDL_BUTTON_MMASK) > 0,
            (mask & SDL_BUTTON_RMASK) > 0};
}

Mouse& Mouse::setCursor(Mouse::Cursor cursor)
{
    const int index = int(cursor);
    if (index >= 0 && index < int(d->cursors.size()))
        SDL_SetCursor(d->cursors.at(index));

    return *this;
}

} // namespace platform
} // namespace pt
