#pragma once

#include <utility>
#include <vector>
#include <array>

#include "geometry.h"
#include "image.h"

namespace hc
{

struct ImageCube
{
    enum Side
    {
        Front, Back, Left, Right, Top, Bottom
    };

    ImageCube(const std::string& filename, int depth = 0);

    const Image& side(Side s) const;

    int width() const;
    int height() const;
    int depth() const;

    std::vector<Image> sides;
};

template <typename T>
using CubeRect = std::array<Rect<T>, 6>;

} // namespace
