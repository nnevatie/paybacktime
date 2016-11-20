#pragma once

#include <memory>
#include <string>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "common/file_system.h"
#include "geom/grid.h"

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

    Model       model()     const;
    std::string name()      const;

    float       scale()     const;
    glm::vec3   origin()    const;
    glm::mat4x4 transform() const;

    glm::vec3 dimensions() const;

    bool transparent() const;
    Object& updateTransparency();

    Grid<float> density() const;
    Object& updateDensity();

    Grid<glm::vec3> emission() const;
    Grid<glm::vec4> bleed() const;
    Object& updateMaterial();

    Object flipped(TextureStore* textureStore) const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
