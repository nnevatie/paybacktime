#pragma once

namespace pt
{
namespace cfg
{

struct Video
{
    float renderScale;
};

namespace preset
{

static constexpr Video ULTRA = {1.00f},
                       HIGH  = {0.75f},
                       LOW   = {0.50f};

} // namespace preset

} // namespace cfg
} // namespace pt
