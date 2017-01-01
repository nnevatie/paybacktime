#pragma once

#include <memory>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "geom/transform.h"
#include "gl/texture.h"

#include "scene/material_types.h"
#include "scene/horizon.h"

namespace pt
{
namespace gfx
{

struct Lightmapper
{
    Lightmapper();

    gl::Texture* lightTexture() const;
    gl::Texture* incidenceTexture() const;

    Lightmapper& reset(const glm::ivec3& size = {});

    Lightmapper& add(const glm::ivec3& pos,
                     const Rotation& rot,
                     const mat::Density& density,
                     const mat::Emission& emission);

    Lightmapper& operator()(const Horizon& horizon);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gfx
} // namespace pt
