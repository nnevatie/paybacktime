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
    static const int  MIPMAP_COUNT_MAX = 8;

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
                      fsSsrComposite;

    gl::ShaderProgram progScale,
                      progBlur,
                      progSsr,
                      progSsrComposite;

    gl::Texture       texScale[MIPMAP_COUNT_MAX],
                      texBlur[MIPMAP_COUNT_MAX],
                      texColor,
                      texSsr;

    gl::Fbo           fboScale[MIPMAP_COUNT_MAX],
                      fboBlur[MIPMAP_COUNT_MAX],
                      fboColor,
                      fboSsr;

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
