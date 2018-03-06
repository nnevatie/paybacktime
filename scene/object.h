#pragma once

#include <memory>
#include <string>
#include <utility>
#include <functional>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "common/file_system.h"
#include "geom/transform.h"

#include "material_types.h"
#include "texture_store.h"
#include "model.h"
#include "state.h"

namespace pt
{
struct Object;
using Objects = std::vector<Object>;

struct Object
{
    // ID
    using Id  = std::string;
    using Ids = std::vector<Id>;

    // Resolver
    using Resolver = std::function<Object(const Id& id,
                                          TextureStore& textureStore)>;
    // Object path, store root
    using Path = std::pair<fs::path, fs::path>;

    Object() = default;
    Object(const glm::vec3& size);
    Object(const Path& path,
           const Resolver& resolver,
           TextureStore& textureStore);

    operator bool() const;

    bool operator==(const Object& other) const;
    bool operator!=(const Object& other) const;

    Object clone() const;
    bool detach();

    Object parent() const;
    Object& setParent(const Object& object);

    Objects hierarchy() const;

    Model model() const;
    State& state() const;

    std::string id() const;
    std::string name() const;

    glm::vec3 origin() const;

    glm::vec3 dimensions() const;

    bool transparent() const;
    Object& updateTransparency();

    mat::Density& density() const;
    Object& updateDensity();

    bool emissive() const;
    mat::Emission& emission() const;
    Object& updateEmissivity();

    mat::Pulse& pulse() const;

    Object& updateMaterial();

    Object& updateApproximation();

    bool update(const Resolver& resolver, TextureStore& textureStore);

    Object flipped(TextureStore& textureStore) const;

    static Id   pathId(const Path& path);
    static bool exists(const fs::path& path);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
