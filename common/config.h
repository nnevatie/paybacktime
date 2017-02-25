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

struct Debug
{
    bool detailedStats;
};

struct Config
{
    Video video;
    Debug debug;
};

namespace preset
{

static constexpr Video ULTRA = {{1.00f, 1.00f, 1.00f}},
                       HIGH  = {{1.00f, 0.50f, 1.00f}},
                       LOW   = {{0.75f, 0.50f, 0.50f}};

static constexpr Config config = {HIGH,
                                 {true}};

} // namespace preset

} // namespace cfg
} // namespace pt
