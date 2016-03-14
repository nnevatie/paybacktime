#pragma once

#include <string>
#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <SDL2/SDL.h>

#include "geom/size.h"
#include "img/image.h"

struct NVGcontext;
namespace nanogui
{
class Screen;
}

namespace pt
{
namespace platform
{

struct Display
{
    Display(const std::string& title, const Size<int>& size, bool fullscreen);
    ~Display();

    Size<int>         size()    const;
    SDL_Window*       window()  const;
    SDL_Surface*      surface() const;
    NVGcontext*       nanoVg()  const;
    nanogui::Screen*  nanoGui() const;

    bool open();
    bool close();
    bool update();
    bool swap();

    glm::vec3 ndc(const glm::ivec2& p)  const;
    glm::vec4 clip(const glm::ivec2& p) const;

    Display& processEvent(SDL_Event* event);
    Display& renderWidgets();

    Image capture() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
