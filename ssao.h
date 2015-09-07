#pragma once

#include <GL/glew.h>

#include "gl_texture.h"
#include "gl_fbo.h"

namespace hc
{

struct Ssao
{
    gl::Fbo fbo;
    gl::Texture texColor, texNormal, texDepth;

    Ssao(int width, int height);
};

} // namespace hc
