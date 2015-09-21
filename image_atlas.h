#pragma once

#include <memory>
#include <vector>

#include "geometry.h"
#include "image.h"
#include "image_cube.h"

namespace hc
{

struct ImageAtlas
{
    ImageAtlas(const Size<int>& size);

    Image atlas(bool drawNodes = false) const;

    Rect<int> insert(const Image& image);
    CubeRect<int> insert(const ImageCube& imageCube);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace hc
