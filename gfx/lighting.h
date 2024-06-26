#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include "geom/size.h"
#include "geom/aabb.h"

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
                      fsSc,
                      fsOut,
                      fsCommon;

    gl::ShaderProgram progSc,
                      progOut;

    gl::Texture       texSc,
                      texOut;

    gl::Fbo           fboSc,
                      fboOut;

    gfx::Blur         blurSc;

    int               scSampleCount;

    Lighting(const cfg::Video& config, const gl::Texture& texDepth);

    Lighting& sc(gl::Texture* texDepth,
                 gl::Texture* texLightmap,
                 const Camera& camera,
                 const Aabb& bounds);

    Lighting& operator()(
        gl::Texture* texDepth,
        gl::Texture* texNormal,
        gl::Texture* texColor,
        gl::Texture* texLight,
        gl::Texture* texSsao,
        gl::Texture* texLightmap,
        gl::Texture* texIncidence,
        const Camera& camera,
        const Aabb& bounds,
        float time);

    gl::Texture* output();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gfx
} // namespace pt
