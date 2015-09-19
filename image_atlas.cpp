#include "image_atlas.h"

namespace hc
{

Image hc::ImageAtlas::image(int level) const
{
    return level >= 0 && level < int(images.size()) ?
           images.at(level) : Image();
}

ImageAtlas::ImageAtlas(const Size<int> size, int depth) :
    size(size), images(depth)
{
    for (int i = 0; i < depth; ++i)
        images[i] = Image(size, 4);
}

int ImageAtlas::depth() const
{
    return int(images.size());
}

} // namespace hc
