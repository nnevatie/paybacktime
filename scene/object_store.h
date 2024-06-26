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

    fs::path path() const;

    Objects objects() const;

    Object object(int index) const;
    Object object(const Object::Id& id) const;

    int update(TextureStore& textureStore);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
