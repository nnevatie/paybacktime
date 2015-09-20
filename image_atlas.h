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

    Image atlas() const;

    bool insert(const Image& image);
    void insert(const ImageCube& imageCube);

    //Rect<int> find(const Size<int>& s) const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace hc
