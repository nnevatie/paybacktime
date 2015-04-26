#pragma once

#include <utility>
#include <vector>

#include "image.h"

namespace hc
{

struct ImageCube
{
    enum class Side
    {
        Front, Back, Left,  Right, Top, Bottom
    };

    typedef std::pair<Side, Image> SideImage;

    ImageCube(const std::vector<SideImage>& sides);

    std::vector<SideImage> sides;
};

} // namespace
