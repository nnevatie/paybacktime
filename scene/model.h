#pragma once

#include <memory>

#include "common/file_system.h"
#include "gl/texture_atlas.h"

namespace pt
{

struct Model
{
    Model(const fs::path& path, gl::TextureAtlas* atlas);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace
