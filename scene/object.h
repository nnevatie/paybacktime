#pragma once

#include "common/file_system.h"

#include "texture_store.h"
#include "model.h"

namespace pt
{

struct Object
{
    Object(const fs::path& path, TextureStore* textureStore);

    Model model;
};

} // namespace pt
