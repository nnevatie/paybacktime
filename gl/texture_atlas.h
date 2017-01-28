#pragma once

#include <utility>
#include <array>

#include "geom/size.h"
#include "img/image_atlas.h"
#include "gl/texture.h"

namespace pt
{
namespace gl
{

struct TextureAtlas
{
    // Image and texture rect cubes
    typedef std::pair<RectCube<int>, RectCube<float>> EntryCube;

    TextureAtlas(const Size<int>& size, bool srgb, int margin = 0);

    void update();

    EntryCube insert(const ImageCube& imageCube);
    TextureAtlas& remove(const EntryCube& entry);

    ImageAtlas  atlas;
    gl::Texture texture;
    bool        srgb;
    int         margin;
};

bool valid(const TextureAtlas::EntryCube& entry);

} // namespace gl
} // namespace pt
