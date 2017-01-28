#include "texture_atlas.h"

#include "platform/clock.h"
#include "common/log.h"

namespace pt
{
namespace gl
{

TextureAtlas::TextureAtlas(const Size<int>& size, bool srgb, int margin) :
    atlas(size), srgb(srgb), margin(margin)
{
    // Update initial texture
    update();
}

void TextureAtlas::update()
{
    texture.bind().alloc(atlas.image(), srgb)
                  .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                  .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
                  .set(GL_TEXTURE_MAX_ANISOTROPY_EXT, Texture::anisotropyMax());
}

TextureAtlas::EntryCube TextureAtlas::insert(const ImageCube& imageCube)
{
    EntryCube entryCube;
    const Size<int> size(atlas.size());
    for (std::size_t i = 0; i < imageCube.sides.size(); ++i)
    {
        const Rect<int> ri   = atlas.insert(imageCube.side(ImageCube::Side(i)),
                                            margin);
        const Rect<float> rt = ri.extended(-margin, -margin)
                                 .as<Rect<float>>().scaled(1.f / size.w,
                                                           1.f / size.h);
        entryCube.first[i]   = ri;
        entryCube.second[i]  = rt;
    }
    // Update texture. TODO: Only update dirty regions.
    update();
    return entryCube;
}

TextureAtlas& TextureAtlas::remove(const TextureAtlas::EntryCube& entry)
{
    for (const auto& side : entry.first)
        atlas.remove(side);

    return *this;
}

bool valid(const TextureAtlas::EntryCube& entry)
{
    return entry.first[0].size;
}

} // namespace gl
} // namespace pt
