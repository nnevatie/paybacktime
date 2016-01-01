#pragma once

#include <memory>
#include <vector>

#include "geom/rect.h"
#include "img/image_cube.h"
#include "img/image.h"

namespace hc
{

struct ImageAtlas
{
    ImageAtlas(const Size<int>& size);

    Size<int> size() const;
    Image image(bool drawNodes = false) const;

    Rect<int> insert(const Image& image, int margin = 0);
    RectCube<int> insert(const ImageCube& imageCube, int margin = 0);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace hc
