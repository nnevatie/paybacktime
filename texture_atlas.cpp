#include "texture_atlas.h"

#include "clock.h"
#include "log.h"

namespace hc
{

TextureAtlas::TextureAtlas(const Size<int>& size) :
    atlas(size)
{
    // Update initial texture
    update();
}

void TextureAtlas::update()
{
    texture.bind().alloc(atlas.image())
                  .set(GL_TEXTURE_MIN_FILTER, GL_NEAREST)
                  .set(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

TextureAtlas::EntryCube TextureAtlas::insert(const ImageCube& imageCube)
{
    EntryCube entryCube;
    const Size<int> size(atlas.size());
    for (std::size_t i = 0; i < imageCube.sides.size(); ++i)
    {
        const Rect<int> ri   = atlas.insert(imageCube.side(ImageCube::Side(i)));
        const Rect<float> rt = ri.as<Rect<float>>().scaled(1.f / size.w,
                                                           1.f / size.h);
        entryCube.first[i]   = ri;
        entryCube.second[i]  = rt;
    }
    // Update texture. TODO: Add clamp margins for bilinear filtering.
    //                       Only update dirty regions.
    update();
    return entryCube;
}

} // namespace hc
