#pragma once

#include <memory>
#include <array>

#include <glm/vec2.hpp>

#include <SDL2/SDL.h>

namespace pt
{
namespace platform
{

struct Mouse
{
    typedef std::array<int, 3> Buttons;

    enum class Cursor
    {
        Arrow,
        ArrowWait,
        Crosshair,
        Hand,
        Size
    };

    Mouse();

    glm::ivec2 position() const;
    Buttons buttons() const;
    int wheel() const;

    Mouse& setCursor(Cursor cursor);

    Mouse& reset();
    Mouse& update(const SDL_Event& event);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
