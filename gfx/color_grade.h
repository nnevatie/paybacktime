#pragma once

#include "geom/rect.h"
#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

namespace hc
{
namespace gfx
{

struct ColorGrade
{
    Size<int>         renderSize;
    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      fsColorGrade;

    gl::ShaderProgram prog;

    gl::Texture       tex;
    gl::Fbo           fbo;

    ColorGrade(const Size<int>& renderSize);

    ColorGrade& operator()(gl::Texture* texColor, gl::Texture* texBloom);

    gl::Texture* output();
};

} // namespace gfx
} // namespace hc
