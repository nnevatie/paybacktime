#pragma once

#include <vector>

#include "geometry.h"
#include "image.h"
#include "image_cube.h"

namespace hc
{

struct ImageAtlas
{
    ImageAtlas(const Size<int>& size, int depth = 1);

    int depth() const;

    Image layer(int index = 0) const;

    void insert(const Image& image);
    void insert(const ImageCube& imageCube);

    //Rect<int> find(const Size<int>& s) const;

    const Size<int> size;
    std::vector<Image> layers;
};

} // namespace hc
