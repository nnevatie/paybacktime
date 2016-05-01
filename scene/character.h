#pragma once

#include <memory>

#include "common/file_system.h"

#include "texture_store.h"

namespace pt
{

struct Character
{
    Character();
    Character(const fs::path& path, TextureStore* textureStore);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
