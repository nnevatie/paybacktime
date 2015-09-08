#pragma once

#include <GL/glew.h>

#include "geometry.h"
#include "gl_texture.h"
#include "gl_fbo.h"

namespace hc
{

struct Ssao
{
    gl::Fbo fbo;
    gl::Texture texColor, texNormal, texDepth;

    Ssao(const Size<int>& size);
};

} // namespace hc
