#pragma once

#include <glm/mat4x4.hpp>

#include "geom/size.h"
#include "geom/box.h"

#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

#include "scene/camera.h"
#include "common/config.h"

#include "blur.h"

namespace pt
{
namespace gfx
{

struct Lighting
{
    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      fsGi,
                      fsSc,
                      fsOut,
                      fsCommon;

    gl::ShaderProgram progGi,
                      progSc,
                      progOut;

    gl::Texture       texGi,
                      texSc,
                      texOut;

    gl::Fbo           fboGi,
                      fboSc,
                      fboOut;

    gfx::Blur         blurGi,
                      blurSc;

    int               scSampleCount;

    Lighting(const cfg::Video& config, const gl::Texture& texDepth);

    Lighting& operator()(
        gl::Texture* texDepth,
        gl::Texture* texNormal,
        gl::Texture* texColor,
        gl::Texture* texLight,
        gl::Texture* texSsao,
        gl::Texture* texLightmap,
        gl::Texture* texIncidence,
        const Camera& camera,
        const Box& bounds,
        const glm::mat4& v,
        const glm::mat4& p);

    gl::Texture* output();
};

} // namespace gfx
} // namespace pt
