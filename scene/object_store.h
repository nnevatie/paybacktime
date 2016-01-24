#pragma once

#include <memory>
#include <string>

#include "common/file_system.h"

#include "texture_store.h"
#include "object.h"

namespace pt
{

struct ObjectStore
{
    ObjectStore(const fs::path& path, TextureStore* textureStore);

    Object* object(const std::string& name) const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
