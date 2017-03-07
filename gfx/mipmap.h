#pragma once

#include <memory>

#include "geom/size.h"
#include "gl/texture.h"

namespace pt
{
namespace gfx
{

struct Mipmap
{
    Mipmap(const Size<int>& size, int depth, bool bilateral = false);

    Mipmap& operator()(gl::Texture* tex, gl::Texture* texDepth = nullptr);

    gl::Texture* output();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gfx
} // namespace pt
