#pragma once

#include <utility>
#include <array>

#include "geometry.h"
#include "image_atlas.h"
#include "gl/texture.h"

namespace hc
{
namespace gl
{

struct TextureAtlas
{
    // Image and texture rect cubes
    typedef std::pair<RectCube<int>, RectCube<float>> EntryCube;

    TextureAtlas(const Size<int>& size);

    void update();

    EntryCube insert(const ImageCube& imageCube);

    ImageAtlas  atlas;
    gl::Texture texture;
};

} // namespace gl
} // namespace hc
