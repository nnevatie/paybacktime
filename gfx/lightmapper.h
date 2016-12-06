#pragma once

#include "gl/primitive.h"
#include "gl/shaders.h"

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

    Lightmapper();

    Lightmapper& operator()();
};

} // namespace gfx
} // namespace pt
