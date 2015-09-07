#include "ssao.h"

namespace hc
{

Ssao::Ssao(int width, int height)
{
    auto fboSize = {width, height};
    texColor.bind().alloc(fboSize,  GL_RGB, GL_RGB);
    texNormal.bind().alloc(fboSize, GL_RGB, GL_RGB);
    texDepth.bind().alloc(fboSize,  GL_DEPTH_COMPONENT32F,
                                    GL_DEPTH_COMPONENT, GL_FLOAT);
}

} // namespace hc
