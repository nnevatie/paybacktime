#pragma once

#include <memory>

#include <glm/vec3.hpp>

#include "common/file_system.h"
#include "gl/primitive.h"

#include "texture_store.h"

namespace pt
{

struct Model
{
    Model(const fs::path& path, TextureStore* textureStore);

    glm::vec3 dimensions() const;

    gl::Primitive primitive() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace
