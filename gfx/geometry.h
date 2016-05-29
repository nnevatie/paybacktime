#pragma once

#include <utility>
#include <vector>

#include <glm/mat4x4.hpp>

#include "geom/size.h"
#include "geom/box.h"

#include "gl/primitive.h"
#include "gl/shaders.h"
#include "gl/texture.h"
#include "gl/fbo.h"

#include "scene/camera.h"

namespace pt
{
namespace gfx
{

struct Geometry
{
    static const int  backDownscale = 4;

    Size<int>         renderSize;
    gl::Primitive     rect;

    gl::Shader        vsQuad,
                      vsGeometry,
                      vsGeometryTransparent,
                      gsWireframe,
                      fsGeometry,
                      fsGeometryTransparent,
                      fsOitComposite,
                      fsDenoise,
                      fsCommon;

    gl::ShaderProgram progGeometry,
                      progGeometryTransparent,
                      progOitComposite,
                      progDenoise;

    gl::Texture       texDepth,
                      texDepthBack,
                      texNormal,
                      texNormalDenoise,
                      texColor,
                      texLight,
                      texOit0,
                      texOit1;

    gl::Fbo           fbo,
                      fboBack,
                      fboOit;

    typedef std::pair<gl::Primitive, glm::mat4> Instance;
    typedef std::vector<Instance>               Instances;

    Geometry(const Size<int>& renderSize);

    // Opaque
    Geometry& operator()(gl::Texture* texAlbedo,
                         gl::Texture* texLightmap,
                         const Instances& instances,
                         const glm::mat4& v,
                         const glm::mat4& p);

    // Transparent
    Geometry& operator()(gl::Fbo* fbo,
                         gl::Texture* texAlbedo,
                         gl::Texture* texLightmap,
                         gl::Texture* texGi,
                         gl::Texture* texIncid,
                         const Box& bounds,
                         const Instances& instances,
                         const Camera& camera);
};

} // namespace gfx
} // namespace pt
