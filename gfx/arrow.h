#pragma once

#include <glm/mat4x4.hpp>

#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/fbo.h"

namespace pt
{
namespace gfx
{

struct Arrow
{
    gl::Primitive     arrow;

    gl::Shader        vsArrow,
                      fsArrow;

    gl::ShaderProgram progArrow;

    Arrow();

    Arrow& operator()(gl::Fbo* fboOut, const glm::mat4& mvp);
};

} // namespace gfx
} // namespace pt
