#pragma once

#include <vector>

#include "geom/geometry.h"
#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

namespace hc
{
namespace gfx
{

struct Bloom
{
    static const int  scaleCount = 3;

    Size<int>         renderSize;

    gl::Primitive     rect;

    gl::Shader        vsQuadUv,
                      fsBloom,
                      fsTexture,
                      fsAdd,
                      fsGaussian;

    gl::ShaderProgram progBloom,
                      progScale,
                      progAdd,
                      progBlur;

    gl::Texture       texBloom,
                      texScale[scaleCount],
                      texAdd[scaleCount],
                      texBlur[scaleCount];

    gl::Fbo           fboBloom,
                      fboScale[scaleCount],
                      fboAdd[scaleCount],
                      fboBlur[scaleCount];

    Bloom(const Size<int>& renderSize);

    Bloom& operator()(gl::Texture* texAlbedo,
                      gl::Texture* texColor,
                      gl::Texture* texLight);

    gl::Texture* output();
};

} // namespace gfx
} // namespace hc
