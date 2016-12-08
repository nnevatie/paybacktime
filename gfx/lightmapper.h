#pragma once

#include <glm/vec3.hpp>

#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"

#include "scene/material_types.h"

namespace pt
{
namespace gfx
{

struct Lightmapper
{
    gl::Primitive     rect;

    gl::Shader        vsQuadUv,
                      fsLightmapper;

    gl::ShaderProgram prog;

    mat::Density      density;
    mat::Emission     emission;

    gl::Texture       texLight,
                      texIncidence,
                      texDensity,
                      texEmission;
    Lightmapper();

    Lightmapper& reset(const glm::ivec3& size = {});

    Lightmapper& add(const glm::ivec3& pos,
                     const mat::Density& density,
                     const mat::Emission& emission);

    Lightmapper& operator()();
};

} // namespace gfx
} // namespace pt
