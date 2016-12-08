#include "lightmapper.h"

#include "gl/fbo.h"

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

Lightmapper& Lightmapper::operator()(mat::Light& lightmap,
                                     const mat::Density& density,
                                     const mat::Emission& emission)
{
    if (lightmap)
    {
        tex.bind().alloc(lightmap);

        gl::Fbo fbo;
        Binder<gl::Fbo> fboBinder(&fbo);
        Binder<gl::ShaderProgram> progBinder(&prog);
        prog.setUniform("density", 0);

        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);

        for (int z = 0; z < lightmap.size.z; ++z)
        {
            fbo.attach(tex, gl::Fbo::Attachment::Color, 0, 0, z);
            prog.setUniform("z", 0.f);
            rect.render();

            glReadPixels(0, 0, lightmap.size.x, lightmap.size.y,
                         GL_RGB, GL_FLOAT, static_cast<GLvoid*>(
                                               &lightmap.at(0, 0, z)));
        }
    }
    return *this;
}

} // namespace gfx
} // namespace pt
