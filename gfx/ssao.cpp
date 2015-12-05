#include "ssao.h"

#include <boost/algorithm/clamp.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/random.hpp>
#include <glm/gtx/compatibility.hpp>

namespace hc
{
namespace gfx
{

namespace
{

std::vector<glm::vec3> kernelData(int size)
{
    using namespace boost::algorithm;
    std::vector<glm::vec3> data(size);
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
           const Size<int>& noiseSize) :
    kernelSize(kernelSize),
    renderSize(renderSize),
    noiseSize(noiseSize),
    kernel(kernelData(kernelSize))
{
    auto fboSize  = {renderSize.w, renderSize.h};

    // Alloc depth, normal, color and lighting textures
    texDepth.bind().alloc(fboSize,         GL_DEPTH_COMPONENT32F,
                                           GL_DEPTH_COMPONENT, GL_FLOAT);
    texNormal.bind().alloc(fboSize,        GL_RGB16F,  GL_RGB, GL_FLOAT);
    texNormalDenoise.bind().alloc(fboSize, GL_RGB16F,  GL_RGB, GL_FLOAT);
    texColor.bind().alloc(fboSize,         GL_RGB8,    GL_RGB, GL_UNSIGNED_BYTE);
    texLight.bind().alloc(fboSize,         GL_RGB8,    GL_RGB, GL_UNSIGNED_BYTE);
    texAo.bind().alloc(fboSize,            GL_R16F,    GL_RGB, GL_FLOAT);
    texAoBlur.bind().alloc(fboSize,        GL_R16F,    GL_RGB, GL_FLOAT);
    texLighting.bind().alloc(fboSize,      GL_RGB16F,  GL_RGB, GL_FLOAT);

    // Alloc and generate noise texture
    texNoise.bind().alloc({noiseSize.w, noiseSize.h},
                          GL_RGB16F, GL_RGB, GL_FLOAT,
                          noiseData(noiseSize.area()).data())
                   .set(GL_TEXTURE_WRAP_S, GL_REPEAT)
                   .set(GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Attach textures to FBOs
    fboGeometry.bind()
               .attach(texDepth,         gl::Fbo::Attachment::Depth)
               .attach(texNormal,        gl::Fbo::Attachment::Color, 0)
               .attach(texColor,         gl::Fbo::Attachment::Color, 1)
               .attach(texLight,         gl::Fbo::Attachment::Color, 2)
               .attach(texNormalDenoise, gl::Fbo::Attachment::Color, 3)
               .unbind();

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

} // namespace gfx
} // namespace hc
