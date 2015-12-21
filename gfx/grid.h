#pragma once

#include <glm/mat4x4.hpp>

#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

namespace hc
{
namespace gfx
{

struct Grid
{
    gl::Primitive     grid;

    gl::Shader        vsModel,
                      fsColor;

    gl::ShaderProgram prog;

    Grid();

    Grid& operator()(gl::Fbo* fboOut, const glm::mat4& mvp);
};

} // namespace gfx
} // namespace hc
