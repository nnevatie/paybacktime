#pragma once

#include <glm/mat4x4.hpp>

#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

#include "scene/camera.h"

namespace pt
{
namespace gfx
{

struct Grid
{
    gl::Primitive     grid;
    gl::Primitive     rect;

    gl::Shader        vsModel,
                      fsColor;

    gl::ShaderProgram prog;

    Grid();

    Grid& operator()(gl::Fbo* fboOut, gl::Texture* texDepth,
                     const Camera& camera);
};

} // namespace gfx
} // namespace pt
