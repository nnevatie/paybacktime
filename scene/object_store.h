#pragma once

#include <memory>

#include "common/file_system.h"
#include "object.h"

namespace pt
{

struct ObjectStore
{
    ObjectStore(const fs::path& path, gl::TextureAtlas* atlas);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
