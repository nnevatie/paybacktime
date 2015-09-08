#include "ssao.h"

namespace hc
{

Ssao::Ssao(const Size<int>& size)
{
    auto fboSize = {size.w, size.h};
    texColor.bind().alloc(fboSize,  GL_RGB, GL_RGB);
    texNormal.bind().alloc(fboSize, GL_RGB, GL_RGB);
    texDepth.bind().alloc(fboSize,  GL_DEPTH_COMPONENT32F,
                                    GL_DEPTH_COMPONENT, GL_FLOAT);
}

} // namespace hc
