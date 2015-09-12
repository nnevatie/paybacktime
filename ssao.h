#pragma once

#include <GL/glew.h>

#include "geometry.h"
#include "gl_texture.h"
#include "gl_fbo.h"

namespace hc
{

struct Ssao
{
    Size<int> size,
              noiseSize;

    gl::Fbo fbo;
    gl::Texture texColor, texNormal, texDepth, texNoise;

    Ssao(const Size<int>& size, const Size<int>& noiseSize);
};

} // namespace hc
