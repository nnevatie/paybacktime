#pragma once

#include <vector>

#include "object.h"

namespace pt
{

struct ObjectStore
{
    ObjectStore(const filesystem::path& path, gl::TextureAtlas* atlas);

    std::vector<Object> objects;
};

} // namespace pt
