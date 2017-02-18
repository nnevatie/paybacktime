#pragma once

#include "geom/size.h"
#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"

namespace pt
{
namespace gfx
{

struct Output
{
    Size<int>         renderSize;

    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      fsTex;

    gl::ShaderProgram prog;

    Output(const Size<int>& renderSize);

    Output& operator()(gl::Texture* tex);
};

} // namespace gfx
} // namespace pt
