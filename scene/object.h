#pragma once

#include "common/file_system.h"
#include "gl/texture_atlas.h"

namespace pt
{

struct Object
{
    Object(const filesystem::path& path, gl::TextureAtlas* atlas);
};

} // namespace pt
