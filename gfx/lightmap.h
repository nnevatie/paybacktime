#pragma once

#include <utility>
#include <memory>
#include <vector>
#include <array>

#include <glm/vec3.hpp>

#include "geom/size.h"
#include "gl/texture.h"
#include "scene/material_types.h"
#include "scene/horizon.h"
#include "scene/camera.h"

namespace pt
{
namespace gfx
{

struct Lightmap
{
    // Size
    using Size = glm::ivec3;

    // Normal/high quality textures
    using Tex = std::pair<gl::Texture, gl::Texture>;

    // Emitter (x, y, z, w)
    using Emitter  = std::array<int16_t, 4>;
    using Emitters = std::vector<Emitter>;

    Lightmap();

    Tex& light() const;
    Tex& incidence() const;

    Size size() const;
    Lightmap& resize(const Size& size);

    Lightmap& update(const mat::Density& density,
                     const mat::Emission& emission,
                     const Emitters& emitters,
                     const Horizon& horizon);

    gl::Texture& debug(gl::Texture* texDepth,
                       const pt::Size<int>& size,
                       const Camera& camera,
                       const Aabb& bounds);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gfx
} // namespace pt
