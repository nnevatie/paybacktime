#pragma once

#include <glm/mat4x4.hpp>

#include "geom/size.h"
#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

#include "scene/camera.h"

namespace pt
{
namespace gfx
{

struct Backdrop
{
    Size<int>         renderSize;
    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      fsBackdrop;

    gl::ShaderProgram prog;

    gl::Texture       tex;

    Backdrop(const Size<int>& renderSize);

    Backdrop& operator()(gl::Fbo* fboOut, const Camera& camera);
};

} // namespace gfx
} // namespace pt
