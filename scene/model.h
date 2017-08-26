#pragma once

#include <memory>

#include <glm/vec3.hpp>

#include "common/file_system.h"
#include "img/image.h"
#include "gl/primitive.h"

#include "texture_store.h"

namespace pt
{

struct Model
{
    Model();
    Model(const fs::path& path,
          const Model& base,
          TextureStore& textureStore,
          int smoothness,
          float scale = 1.f);

    operator bool() const;

    glm::vec3 dimensions() const;

    gl::Primitive primitive() const;

    const ImageCube* depthCube()  const;
    const ImageCube* albedoCube() const;
    const ImageCube* lightCube()  const;

    bool update(const Model& base, TextureStore& textureStore);

    Model flipped(TextureStore& textureStore) const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace
