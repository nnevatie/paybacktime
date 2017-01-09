#pragma once

#include <memory>
#include <string>
#include <utility>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "common/file_system.h"

#include "material_types.h"
#include "texture_store.h"
#include "model.h"

namespace pt
{

struct Object
{
    // Object path, store root
    using Path = std::pair<fs::path, fs::path>;

    Object();
    Object(const Path& path, TextureStore* textureStore);

    operator bool() const;

    operator==(const Object& other) const;
    operator!=(const Object& other) const;

    Model       model()     const;
    std::string name()      const;

    float       scale()     const;
    glm::vec3   origin()    const;
    glm::mat4x4 transform() const;

    glm::vec3 dimensions() const;

    bool transparent() const;
    Object& updateTransparency();

    mat::Density density() const;
    Object& updateDensity();

    bool emissive() const;
    mat::Emission emission() const;
    Object& updateEmissivity();

    Object& updateMaterial();

    Object flipped(TextureStore* textureStore) const;

    static bool exists(const fs::path& path);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
