#include "image_atlas.h"

namespace hc
{

Image hc::ImageAtlas::layer(int index) const
{
    return index >= 0 && index < int(layers.size()) ?
                layers.at(index) : Image();
}

ImageAtlas::ImageAtlas(const Size<int>& size, int depth) :
    size(size), layers(depth)
{
    for (int i = 0; i < depth; ++i)
        layers[i] = Image(size, 4);
}

int ImageAtlas::depth() const
{
    return int(layers.size());
}

void ImageAtlas::insert(const Image& image)
{
}

void ImageAtlas::insert(const ImageCube& imageCube)
{
    for (const Image& image : imageCube.sides)
        insert(image);
}

} // namespace hc
