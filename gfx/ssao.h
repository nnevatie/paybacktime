#pragma once

#include <vector>

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include "geom/size.h"
#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

#include "blur.h"

namespace pt
{
namespace gfx
{

struct Ssao
{
    typedef std::vector<glm::vec3> Kernel;

    int               kernelSize;
    Size<int>         displaySize,
                      renderSize,
                      noiseSize;

    Kernel            kernel;

    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      fsCommon,
                      fsAo;

    gl::ShaderProgram progAo;

    gl::Fbo           fboAo;

    gl::Texture       texAo,
                      texNoise;

    Blur              blur;

    Ssao(int kernelSize, const Size<int>& displaySize,
                         const Size<int>& renderSize);

    glm::vec2 noiseScale() const;

    Ssao& operator()(gl::Texture* texDepth,
                     gl::Texture* texNormal,
                     const glm::mat4& proj,
                     float fov);

    gl::Texture& output();
};

} // namespace gfx
} // namespace pt
