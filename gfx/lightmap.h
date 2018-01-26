#pragma once

#include <memory>

namespace pt
{
namespace gfx
{

struct Lightmap
{
    Lightmap();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gfx
} // namespace pt
