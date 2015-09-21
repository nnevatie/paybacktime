#pragma once

#include <array>

#include "geometry.h"
#include "image_atlas.h"
#include "gl_texture.h"

namespace hc
{

struct TextureAtlas
{
    struct Tile
    {
        // Image and texture rects
        Rect<int>   ri;
        Rect<float> rt;
    };
    typedef std::array<Tile, 6> TileCube;

    TextureAtlas(const Size<int>& size);

    void update();

    TileCube insert(const ImageCube& imageCube);

    ImageAtlas  atlas;
    gl::Texture texture;
};

} // namespace hc
