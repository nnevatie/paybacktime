#pragma once

#include <memory>

#include "geom/size.h"
#include "gl/texture.h"

namespace pt
{
namespace gfx
{

struct Erode
{
    Erode(const Size<int>& size);

    Erode& operator()(gl::Texture* tex, int radius);

    gl::Texture& output();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gfx
} // namespace pt
