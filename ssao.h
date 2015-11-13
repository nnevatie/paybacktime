#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/vec2.hpp>

#include "geometry.h"
#include "gl_texture.h"
#include "gl_rbo.h"
#include "gl_fbo.h"

namespace hc
{

struct Ssao
{
    int         kernelSize;
    Size<int>   renderSize,
                noiseSize;

    gl::Fbo     fboGeometry,
                fboEmissive,
                fboEmissiveBlur1,
                fboEmissiveBlur2,
                fboAo,
                fboAoBlur,
                fboOutput;

    gl::Texture texDepth,
                texNormal,
                texNormalDenoise,
                texColor,
                texLight,
                texEmissive,
                texEmissiveBlur,
                texAo,
                texAoBlur,
                texLighting,
                texNoise;

    std::vector<glm::vec3> kernel;

    Ssao(int kernelSize,
         const Size<int>& renderSize,
         const Size<int>& noiseSize);

    glm::vec2 noiseScale() const;
};

} // namespace hc
