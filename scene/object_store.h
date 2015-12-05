#pragma once

#include <vector>

#include "object.h"

namespace hc
{

struct ObjectStore
{
    ObjectStore(const filesystem::path& path, gl::TextureAtlas* atlas);

    std::vector<Object> objects;
};

} // namespace hc
