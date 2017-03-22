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
    Size<int>         displaySize;
    Size<int>         renderSize;
    float             scale;

    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      fsCommon,
                      fsSsr,
                      fsComp;

    gl::ShaderProgram progSsr,
                      progComp;

    gl::Texture       texSsrUva,
                      texComp;

    gl::Fbo           fboSsr,
                      fboComp;

    Ssr(const Size<int>& displaySize, const Size<int>& renderSize);

    Ssr& operator()(gl::Texture* texDepth,
                    gl::Texture* texNormal,
                    gl::Texture* texLight,
                    gl::Texture* texColor,
                    gl::Texture* texEnv,
                    const Camera& camera);

    gl::Texture* output();
};

} // namespace gfx
} // namespace pt
