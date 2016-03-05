#pragma once

#include <memory>
#include <string>

#include "common/file_system.h"

#include "texture_store.h"
#include "model.h"

namespace pt
{

struct Object
{
    Object();
    Object(const fs::path& path, TextureStore* textureStore);

    operator bool() const;

    operator==(const Object& other) const;
    operator!=(const Object& other) const;

    std::string name()  const;
    Model       model() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
