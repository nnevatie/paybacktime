#pragma once

#include <vector>

#include "geometry.h"
#include "image.h"

namespace hc
{

struct ImageAtlas
{
    ImageAtlas(const Size<int>& size, int depth = 1);

    int depth() const;

    Image image(int level = 0) const;

    const Size<int> size;
    std::vector<Image> images;
};

} // namespace hc
