#include "ssao.h"

#include <boost/algorithm/clamp.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/compatibility.hpp>

#include "common/common.h"
#include "common/log.h"

namespace pt
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
           const Size<int>& displaySize,
           const Size<int>& renderSize,
           const Size<int>& noiseSize) :
    kernelSize(kernelSize),
    displaySize(displaySize),
    renderSize(renderSize),
    noiseSize(noiseSize),
    kernel(kernelData(kernelSize)),
    rect(squareMesh()),
    vsQuad(gl::Shader::path("quad_uv.vs.glsl")),
    fsCommon(gl::Shader::path("common.fs.glsl")),
    fsAo(gl::Shader::path("ssao.fs.glsl")),
    progAo({vsQuad, fsAo, fsCommon},
          {{0, "position"}, {1, "uv"}}),
    blur(renderSize)
{
    auto fboSize = {renderSize.w, renderSize.h};

    // Alloc textures
    texAo.bind().alloc(fboSize, GL_R8, GL_RED, GL_UNSIGNED_BYTE);

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
}

glm::vec2 Ssao::noiseScale() const
{
    return glm::vec2(float(displaySize.w) / noiseSize.w,
                     float(displaySize.h) / noiseSize.h);
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
                     .setUniform("tanHalfFov",  std::tan(0.5f * fov))
                     .setUniform("aspectRatio", renderSize.aspect<float>());

        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glDisable(GL_DEPTH_TEST);
        glViewport(0, 0, renderSize.w, renderSize.h);

        texDepth->bindAs(GL_TEXTURE0);
        texNormal->bindAs(GL_TEXTURE1);
        texNoise.bindAs(GL_TEXTURE2);
        rect.render();
        blur(&texAo, nullptr, 3);
    }
    return *this;
}

gl::Texture& Ssao::output()
{
    return blur.output();
}

} // namespace gfx
} // namespace pt
