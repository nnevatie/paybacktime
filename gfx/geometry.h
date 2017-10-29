#pragma once

#include <utility>
#include <vector>

#include <glm/mat4x4.hpp>

#include "geom/size.h"
#include "geom/aabb.h"

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
                      fsLinearDepth,
                      fsCommon;

    gl::ShaderProgram progGeometry,
                      progGeometryTransparent,
                      progOitComposite,
                      progDenoise,
                      progLinearDepth;

    gl::Texture       texDepth,
                      texDepthLinear,
                      texNormal,
                      texNormalDenoise,
                      texColor,
                      texLight,
                      texOit0,
                      texOit1,
                      texComp;

    gl::Fbo           fbo,
                      fboOit,
                      fboComp;

    using Instance  = std::pair<gl::Primitive, glm::mat4>;
    using Instances = std::vector<Instance>;

    Geometry(const Size<int>& renderSize);

    // Opaque
    Geometry& operator()(gl::Texture* texAlbedo,
                         gl::Texture* texNormalMap,
                         gl::Texture* texLightmap,
                         const Instances& instances,
                         const Camera& camera);

    // Transparent
    Geometry& operator()(gl::Texture* texOpaque,
                         gl::Texture* texEnv,
                         gl::Texture* texAlbedo,
                         gl::Texture* texLightmap,
                         gl::Texture* texGi,
                         gl::Texture* texIncid,
                         const Aabb& bounds,
                         const Instances& instances,
                         const Camera& camera);
};

} // namespace gfx
} // namespace pt
