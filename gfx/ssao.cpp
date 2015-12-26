#include "ssao.h"

#include <boost/algorithm/clamp.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/random.hpp>
#include <glm/gtx/compatibility.hpp>

#include "common/common.h"

namespace hc
{
namespace gfx
{

namespace
{

Ssao::Kernel kernelData(int size)
{
    using namespace boost::algorithm;
    Ssao::Kernel data(size);
    for (int i = 0; i < size; ++i)
    {
        // Positive Z-axis hemisphere
        // Scaled to include more samples near the center of the kernel
        const float s0  = float(i) / size;
        const float s1  = clamp(s0 * s0, 0.1f, 1.f);
        glm::vec3 v     = glm::sphericalRand<float>(s1);
        data[i]         = glm::vec3(v.x, v.y, std::abs(v.z));
    }
    return data;
}

std::vector<float> noiseData(int size)
{
    std::vector<float> data(3 * size);
    for (int i = 0; i < size; ++i)
    {
        glm::vec2 v     = glm::circularRand<float>(1);
        data[i * 3 + 0] = v.x;
        data[i * 3 + 1] = v.y;
        data[i * 3 + 2] = 0;
    }
    return data;
}

}

Ssao::Ssao(int kernelSize,
           const Size<int>& renderSize,
           const Size<int>& noiseSize,
           const gl::Texture& texDepth) :
    kernelSize(kernelSize),
    renderSize(renderSize),
    noiseSize(noiseSize),
    kernel(kernelData(kernelSize)),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsCommon(gl::Shader::path("common.fs.glsl")),
    fsAo(gl::Shader::path("ssao.fs.glsl")),
    fsBlur(gl::Shader::path("blur.fs.glsl")),
    progAo({vsQuad, fsAo, fsCommon},
          {{0, "position"}, {1, "uv"}}),
    progBlur({vsQuad, fsBlur},
            {{0, "position"}, {1, "uv"}})
{
    auto fboSize  = {renderSize.w, renderSize.h};

    // Alloc textures
    texAo.bind().alloc(fboSize,       GL_R16F,    GL_RGB, GL_FLOAT);
    texAoBlur.bind().alloc(fboSize,   GL_R16F,    GL_RGB, GL_FLOAT);
    texLighting.bind().alloc(fboSize, GL_RGB16F,  GL_RGB, GL_FLOAT);

    // Alloc and generate noise texture
    texNoise.bind().alloc({noiseSize.w, noiseSize.h},
                          GL_RGB16F, GL_RGB, GL_FLOAT,
                          noiseData(noiseSize.area()).data())
                   .set(GL_TEXTURE_WRAP_S, GL_REPEAT)
                   .set(GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Attach textures to FBOs
    fboAo.bind()
         .attach(texAo, gl::Fbo::Attachment::Color)
         .unbind();

    fboAoBlur.bind()
           .attach(texAoBlur, gl::Fbo::Attachment::Color)
           .unbind();

    fboOutput.bind()
             .attach(texDepth,    gl::Fbo::Attachment::Depth)
             .attach(texLighting, gl::Fbo::Attachment::Color)
             .unbind();
}

glm::vec2 Ssao::noiseScale() const
{
    return glm::vec2(float(renderSize.w) / noiseSize.w,
                     float(renderSize.h) / noiseSize.h);
}

Ssao& Ssao::operator()(gl::Texture* texDepth,
                       gl::Texture* texNormal,
                       const glm::mat4& proj,
                       float fov)
{
    {
        // AO pass
        Binder<gl::Fbo> binder(fboAo);
        progAo.bind().setUniform("texDepth",    0)
                     .setUniform("texNormal",   1)
                     .setUniform("texNoise",    2)
                     .setUniform("kernel",      kernel)
                     .setUniform("noiseScale",  noiseScale())
                     .setUniform("p",           proj)
                     .setUniform("tanHalfFov",  std::tan(radians(0.5f * fov)))
                     .setUniform("aspectRatio", renderSize.aspect<float>());

        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);

        texDepth->bindAs(GL_TEXTURE0);
        texNormal->bindAs(GL_TEXTURE1);
        texNoise.bindAs(GL_TEXTURE2);
        rect.render();
    }
    {
        // Blur pass
        Binder<gl::Fbo> binder(fboAoBlur);
        progBlur.bind().setUniform("tex", 0);
        glDisable(GL_DEPTH_TEST);
        texAo.bindAs(GL_TEXTURE0);
        rect.render();
    }
    return *this;
}

} // namespace gfx
} // namespace hc
