#pragma once

#include "common/file_system.h"
#include "gl/texture_atlas.h"

#include "model.h"

namespace pt
{

struct Object
{
    Object(const fs::path& path, gl::TextureAtlas* atlas);

    Model model;
};

} // namespace pt
