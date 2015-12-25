#pragma once

#include <vector>

#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include "geom/size.h"
#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

namespace hc
{
namespace gfx
{

struct Ssao
{
    typedef std::vector<glm::vec3> Kernel;

    int               kernelSize;
    Size<int>         renderSize,
                      noiseSize;

    Kernel            kernel;

    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      fsCommon,
                      fsAo,
                      fsBlur;

    gl::ShaderProgram progAo,
                      progBlur;

    gl::Fbo           fboGeometry,
                      fboAo,
                      fboAoBlur,
                      fboOutput;

    gl::Texture       texDepth,
                      texNormal,
                      texNormalDenoise,
                      texColor,
                      texLight,
                      texAo,
                      texAoBlur,
                      texLighting,
                      texNoise;

    Ssao(int kernelSize,
         const Size<int>& renderSize,
         const Size<int>& noiseSize);

    glm::vec2 noiseScale() const;

    Ssao& operator()(const glm::mat4& proj, float fov);
};

} // namespace gfx
} // namespace hc
