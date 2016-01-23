#pragma once

#include "geom/size.h"

#include "gl/texture_atlas.h"

namespace pt
{

struct TextureStore
{
    TextureStore(const Size<int>& size);

    gl::TextureAtlas albedo;
    gl::TextureAtlas light;
};

} // namespace pt
