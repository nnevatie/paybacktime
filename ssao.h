#pragma once

#include <vector>
#include <GL/glew.h>

#include "geometry.h"
#include "gl_texture.h"
#include "gl_fbo.h"

namespace hc
{

struct Ssao
{
    int       kernelSize;
    Size<int> renderSize,
              noiseSize;

    gl::Fbo fbo;
    gl::Texture texColor, texNormal, texDepth, texNoise;
    std::vector<float> kernel;

    Ssao(int kernelSize,
         const Size<int>& renderSize,
         const Size<int>& noiseSize);
};

} // namespace hc
