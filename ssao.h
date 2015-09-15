#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/vec2.hpp>

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

    gl::Fbo fbo[2];
    gl::Texture texColor,
                texNormal,
                texBlur,
                texDepth,
                texNoise;

    std::vector<float> kernel;

    Ssao(int kernelSize,
         const Size<int>& renderSize,
         const Size<int>& noiseSize);

    glm::vec2 noiseScale() const;
};

} // namespace hc
