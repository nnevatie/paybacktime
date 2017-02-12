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
    using Objects = std::vector<Object>;

    ObjectStore(const fs::path& path, TextureStore& textureStore);

    Objects objects() const;

    Object object(int index) const;
    Object object(const std::string& name) const;

    int update(TextureStore& textureStore);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
