#pragma once

#include <memory>
#include <string>

#include <glm/vec3.hpp>

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

    Model       model()  const;
    std::string name()   const;
    glm::vec3   origin() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
