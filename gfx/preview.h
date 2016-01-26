#pragma once

#include <utility>

#include <glm/mat4x4.hpp>

#include "geom/size.h"
#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

#include "anti_alias.h"

namespace pt
{
namespace gfx
{

struct Preview
{
    Size<int>         renderSize;
    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      vsModel,
                      fsModel,
                      fsDenoise;

    gl::ShaderProgram progModel,
                      progDenoise;

    gl::Texture       texDepth,
                      texColor,
                      texDenoise;

    gl::Fbo           fboModel,
                      fboDenoise;

    AntiAlias         antiAlias;

    Preview(const Size<int>& renderSize);

    Preview& operator()(gl::Texture* texAlbedo,
                        const gl::Primitive& primitive,
                        const glm::mat4& mvp);

    gl::Texture* output();
};

} // namespace gfx
} // namespace pt
