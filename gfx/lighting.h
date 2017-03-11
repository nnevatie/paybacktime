#pragma once

#include <memory>

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

    Lighting& gi(gl::Texture* texDepth,
                 gl::Texture* texLightmap,
                 const Camera& camera,
                 const Box& bounds);

    Lighting& sc(gl::Texture* texDepth,
                 gl::Texture* texLightmap,
                 const Camera& camera,
                 const Box& bounds);

    Lighting& operator()(
        gl::Texture* texDepth,
        gl::Texture* texNormal,
        gl::Texture* texColor,
        gl::Texture* texLight,
        gl::Texture* texSsao,
        gl::Texture* texLightmap,
        gl::Texture* texIncidence,
        const Camera& camera,
        const Box& bounds);

    gl::Texture* output();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gfx
} // namespace pt
