#pragma once

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

    gl::Texture       texLight;

    Lightmapper();

    Lightmapper& operator()(mat::Light& lightmap,
                            const mat::Density& density,
                            const mat::Emission& emission);
};

} // namespace gfx
} // namespace pt
