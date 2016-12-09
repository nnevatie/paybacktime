#pragma once

#include <memory>

#include <glm/vec3.hpp>

#include "gl/texture.h"

#include "scene/material_types.h"

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
                     const mat::Density& density,
                     const mat::Emission& emission);

    Lightmapper& operator()();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gfx
} // namespace pt
