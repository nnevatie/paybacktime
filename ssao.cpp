#include "ssao.h"

#include <vector>
#include <boost/algorithm/clamp.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/random.hpp>

namespace hc
{
namespace
{

std::vector<float> kernelData(int size)
{
    using namespace boost::algorithm;

    std::vector<float> data(3 * size);
    for (int i = 0; i < size; ++i)
    {
        // Positive Z-axis hemisphere
        // Scaled to include more samples near the center of the kernel
        const float s0  = float(i) / size;
        const float s1  = clamp(s0 * s0, 0.1f, 1.f);
        glm::vec3 v     = glm::sphericalRand<float>(s1);
        data[i * 3 + 0] = v.x;
        data[i * 3 + 1] = v.y;
        data[i * 3 + 2] = std::abs(v.z);
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
    auto fboSize = {renderSize.w, renderSize.h};
    texColor.bind().alloc(fboSize,  GL_RGB, GL_RGB);
    texNormal.bind().alloc(fboSize, GL_RGB, GL_RGB);
    texDepth.bind().alloc(fboSize,  GL_DEPTH_COMPONENT32F,
                                    GL_DEPTH_COMPONENT, GL_FLOAT);

    texNoise.bind().alloc({noiseSize.w, noiseSize.h},
                          GL_RGB32F, GL_RGB, GL_FLOAT,
                          noiseData(noiseSize.area()).data());
}

} // namespace hc
