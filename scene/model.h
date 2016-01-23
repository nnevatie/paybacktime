#pragma once

#include <memory>

#include "common/file_system.h"

#include "texture_store.h"

namespace pt
{

struct Model
{
    Model(const fs::path& path, TextureStore* textureStore);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace
