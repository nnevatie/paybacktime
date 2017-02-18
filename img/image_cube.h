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
    enum class Side
    {
        Front, Back, Left, Right, Top, Bottom
    };

    ImageCube();
    ImageCube(const fs::path& path, int depth, bool fallback = true);

    operator bool() const;

    const Image& side(Side s) const;

    int width()  const;
    int height() const;
    int depth()  const;

    bool transparent() const;

    bool emissive() const;

    ImageCube scaled(const ImageCube& refCube) const;

    ImageCube flipped() const;

    ImageCube normals() const;

    int merge(const ImageCube& cube);

    std::vector<Image> sides;
};

template <typename T>
using RectCube = std::array<Rect<T>, 6>;

} // namespace
