#pragma once

namespace pt
{
namespace cfg
{

struct Scales
{
    float render,
          ssao,
          ssr;
};

struct Video
{
    Scales scales;
};

namespace preset
{

static constexpr Video ULTRA = {{1.00f, 1.00f, 1.00f}},
                       HIGH  = {{1.00f, 0.50f, 0.50f}},
                       LOW   = {{0.75f, 0.50f, 0.50f}};

} // namespace preset

} // namespace cfg
} // namespace pt
