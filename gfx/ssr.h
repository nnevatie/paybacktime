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
    static const int  MIPMAP_COUNT_MAX = 4;

    Size<int>         displaySize;
    Size<int>         renderSize;
    float             scale;
    int               mipmapCount;

    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      fsCommon,
                      fsTexture,
                      fsBlur,
                      fsSsr,
                      fsComp;

    gl::ShaderProgram progScale,
                      progBlur,
                      progSsr,
                      progComp;

    gl::Texture       texScale[MIPMAP_COUNT_MAX],
                      texBlur[MIPMAP_COUNT_MAX],
                      texEnv,
                      texSsr,
                      texComp;

    gl::Fbo           fboScale[MIPMAP_COUNT_MAX],
                      fboBlur[MIPMAP_COUNT_MAX],
                      fboEnv,
                      fboSsr,
                      fboComp;

    Ssr(const Size<int>& displaySize, const Size<int>& renderSize);

    Ssr& operator()(gl::Texture* texDepth,
                    gl::Texture* texNormal,
                    gl::Texture* texColor,
                    gl::Texture* texLight,
                    const Camera& camera);

    gl::Texture* output();
};

} // namespace gfx
} // namespace pt
