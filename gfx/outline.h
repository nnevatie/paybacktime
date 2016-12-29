#pragma once

#include <glm/mat4x4.hpp>

#include "geom/size.h"
#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

namespace pt
{
namespace gfx
{

struct Outline
{
    Size<int>         renderSize;

    gl::Primitive     rect;

    gl::Shader        vsModel,
                      fsModel,
                      vsQuad,
                      fsDenoise,
                      fsOutline;

    gl::ShaderProgram progModel,
                      progDenoise,
                      progOutline;

    gl::Texture       texModel,
                      texDenoise;
    gl::Fbo           fboModel,
                      fboDenoise;

    Outline(const Size<int>& renderSize, const gl::Texture& texDepth);

    Outline& operator()(gl::Fbo* fboOut,
                        gl::Texture* texColor,
                        const gl::Primitive& primitive,
                        const glm::mat4& mvp,
                        const glm::vec4& color);
};

} // namespace gfx
} // namespace pt
