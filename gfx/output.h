#pragma once

#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"

namespace hc
{
namespace gfx
{

struct Output
{
    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      fsTex;

    gl::ShaderProgram prog;

    Output();

    Output& operator()(gl::Texture* tex);
};

} // namespace gfx
} // namespace hc
