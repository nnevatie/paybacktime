#pragma once

#include <utility>
#include <vector>

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

struct Geometry
{
    Size<int>         renderSize;
    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      vsGeometry,
                      gsWireframe,
                      fsGeometry,
                      fsDenoise,
                      fsCommon;

    gl::ShaderProgram progGeometry,
                      progDenoise;

    gl::Texture       texDepth,
                      texNormal,
                      texNormalDenoise,
                      texColor,
                      texLight;

    gl::Fbo           fbo;

    typedef std::pair<gl::Primitive, glm::mat4> Instance;
    typedef std::vector<Instance>               Instances;

    Geometry(const Size<int>& renderSize);

    Geometry& operator()(gl::Texture* texAlbedo,
                         gl::Texture* texLightmap,
                         const Instances& instances,
                         const glm::mat4& v,
                         const glm::mat4& p);

    Geometry& operator()(gl::Fbo* fbo,
                         gl::Texture* texAlbedo,
                         gl::Texture* texLightmap,
                         const Instances& instances,
                         const glm::mat4& v,
                         const glm::mat4& p);
};

} // namespace gfx
} // namespace pt
