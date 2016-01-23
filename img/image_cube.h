#pragma once

#include <utility>
#include <vector>
#include <array>

#include "geom/rect.h"
#include "img/image.h"

namespace pt
{

struct ImageCube
{
    enum Side
    {
        Front, Back, Left, Right, Top, Bottom
    };

    ImageCube(const fs::path& path, int depth = 0);

    const Image& side(Side s) const;

    int width()  const;
    int height() const;
    int depth()  const;

    std::vector<Image> sides;
};

template <typename T>
using RectCube = std::array<Rect<T>, 6>;

} // namespace
