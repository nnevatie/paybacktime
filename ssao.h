#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/vec2.hpp>

#include "geometry.h"
#include "gl/gl_texture.h"
#include "gl/gl_rbo.h"
#include "gl/gl_fbo.h"

namespace hc
{

struct Ssao
{
    int         kernelSize;
    Size<int>   renderSize,
                noiseSize;

    gl::Fbo     fboGeometry,
                fboAo,
                fboAoBlur,
                fboOutput;

    gl::Texture texDepth,
                texNormal,
                texNormalDenoise,
                texColor,
                texLight,
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
