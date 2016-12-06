#include "lightmapper.h"

namespace pt
{
namespace gfx
{

Lightmapper::Lightmapper() :
    rect(squareMesh()),
    vsQuadUv(gl::Shader::path("quad_uv.vs.glsl")),
    fsLightmapper(gl::Shader::path("lightmapper.fs.glsl")),
    prog({vsQuadUv, fsLightmapper},
         {{0, "position"}, {1, "uv"}})
{
}

Lightmapper& Lightmapper::operator()()
{
    return *this;
}

} // namespace gfx
} // namespace pt
