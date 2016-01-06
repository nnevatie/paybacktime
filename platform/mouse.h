#pragma once

#include <memory>
#include <array>

#include <glm/vec2.hpp>

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

    Mouse& setCursor(Cursor cursor);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
