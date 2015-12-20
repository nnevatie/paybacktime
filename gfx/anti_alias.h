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

struct AntiAlias
{
    Size<int>         renderSize;
    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      fsFxaa;

    gl::ShaderProgram prog;

    gl::Texture       tex;
    gl::Fbo           fbo;

    AntiAlias(const Size<int>& renderSize);

    AntiAlias& operator()(gl::Texture* texColor);

    gl::Texture* output();
};

} // namespace gfx
} // namespace hc
