#pragma once

#include <array>

#include "image.h"

namespace hc
{

struct ImageCube
{
    typedef std::array<Image, 6> Sides;

    ImageCube(const Sides&& sides);

    // Front, back, left, right, top, bottom
    Sides sides;
};

} // namespace
