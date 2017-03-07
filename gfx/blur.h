#pragma once

#include <memory>

#include "geom/size.h"
#include "gl/texture.h"

namespace pt
{
namespace gfx
{

struct Blur
{
    Blur();

    Blur(const Size<int>& size,
         bool bilateral = false);

    Blur(const Size<int>& size, gl::Texture* out, int level,
         bool bilateral = false);

    Blur& operator()(gl::Texture* tex, gl::Texture* texDepth,
                     int radius, float sharpness = 1.f);

    gl::Texture& output();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gfx
} // namespace pt
