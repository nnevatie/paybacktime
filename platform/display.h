#pragma once

#include <string>
#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <SDL2/SDL.h>

#include "geom/size.h"
#include "img/image.h"

// NanoVC context type
struct NVGcontext;

namespace pt
{
namespace platform
{

struct Display
{
    Display(const std::string& title, const Size<int>& size);
    ~Display();

    Size<int> size() const;

    NVGcontext* nanoVg() const;
    SDL_Surface* surface() const;

    bool open();
    bool close();
    bool update();
    bool swap();

    glm::vec3 rayNdc(const glm::ivec2& p) const;
    glm::vec4 rayClip(const glm::ivec2& p) const;

    Image capture() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
