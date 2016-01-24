#pragma once

#include "gl/primitive.h"
#include "gl/shaders.h"

namespace pt
{
namespace gfx
{

struct Fader
{
    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      fsColor;

    gl::ShaderProgram prog;

    Fader();

    Fader& operator()(float alpha);
};

} // namespace gfx
} // namespace pt
