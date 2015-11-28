#pragma once

#include <vector>

#include "geometry.h"
#include "gl_primitive.h"
#include "gl_shaders.h"
#include "gl_texture.h"
#include "gl_fbo.h"

namespace hc
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

    gl::ShaderProgram bloomProg,
                      scaleProg,
                      addProg,
                      blurProg;

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

} // namespace hc
