#pragma once

#include "geom/size.h"
#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"
#include "scene/camera.h"

namespace pt
{
namespace gfx
{

struct Ssr
{
    static const int  mipmapCount = 8;

    Size<int>         renderSize;
    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      fsCommon,
                      fsSsr;

    gl::ShaderProgram prog;

    gl::Texture       texColor;
    gl::Texture       texSsr;
    gl::Fbo           fbo;

    Ssr(const Size<int>& renderSize);

    Ssr& operator()(gl::Texture* texDepth,
                    gl::Texture* texDepthBack,
                    gl::Texture* texNormal,
                    gl::Texture* texColor,
                    gl::Texture* texLight,
                    const Camera& camera);

    gl::Texture* output();
};

} // namespace gfx
} // namespace pt
