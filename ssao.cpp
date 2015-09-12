#include "ssao.h"

#include <vector>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/random.hpp>

namespace hc
{
namespace
{

std::vector<float> noiseData(int noiseLen)
{
    std::vector<float> noise(3 * noiseLen);
    for (int i = 0; i < noiseLen; ++i)
    {
        glm::vec2 v = glm::circularRand<float>(1);
        noise[i * 3 + 0] = v.x;
        noise[i * 3 + 1] = v.y;
        noise[i * 3 + 2] = 0;
    }
    return noise;
}

}

Ssao::Ssao(const Size<int>& size, const Size<int>& noiseSize) :
    size(size), noiseSize(noiseSize)
{
    auto fboSize = {size.w, size.h};
    texColor.bind().alloc(fboSize,  GL_RGB, GL_RGB);
    texNormal.bind().alloc(fboSize, GL_RGB, GL_RGB);
    texDepth.bind().alloc(fboSize,  GL_DEPTH_COMPONENT32F,
                                    GL_DEPTH_COMPONENT, GL_FLOAT);

    texNoise.bind().alloc({noiseSize.w, noiseSize.h},
                          GL_RGB32F, GL_RGB, GL_FLOAT,
                          noiseData(noiseSize.area()).data());
}

} // namespace hc
