#pragma once

#include "file_system.h"
#include "texture_atlas.h"

namespace hc
{

struct ObjectStore
{
    ObjectStore(const filesystem::path& path, TextureAtlas* atlas);
};

} // namespace hc
