#pragma once

#include <utility>

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

struct Preview
{
    Size<int>         renderSize;
    gl::Primitive     rect;

    gl::Shader        vsModel,
                      fsModel;

    gl::ShaderProgram prog;

    gl::Texture       texDepth,
                      texColor;
    gl::Fbo           fbo;

    Preview(const Size<int>& renderSize);

    Preview& operator()(gl::Texture* texAlbedo,
                        const gl::Primitive& primitive,
                        const glm::mat4& mvp);
};

} // namespace gfx
} // namespace pt
