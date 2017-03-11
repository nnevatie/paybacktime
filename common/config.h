#pragma once

#include <glm/vec2.hpp>

namespace pt
{
namespace cfg
{
namespace video
{

struct Output
{
    float     scale;
    glm::vec2 size;

    glm::vec2 renderSize() const
    {
        return scale * size;
    }
};

struct Gi
{
    float scale;
};

struct Scattering
{
    float scale;
    int   samples;
};

struct Ssao
{
    float scale;
};

struct Env
{
    float scale;
};

struct Ssr
{
    float scale;
};

} // namespace video

struct Video
{
    video::Output     output;
    video::Ssao       ssao;
    video::Env        env;
    video::Ssr        ssr;
    video::Gi         gi;
    video::Scattering sc;
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

static constexpr Video
    ULTRA = {{1.00f}, {1.00f}, {1.00f}, {1.00f}, {1.00f}, {1.00f, 20}},
    HIGH  = {{1.00f}, {0.50f}, {0.50f}, {1.00f}, {1.00f}, {0.50f, 10}},
    LOW   = {{1.00f}, {0.50f}, {0.25f}, {0.50f}, {0.50f}, {0.25f, 10}};

static constexpr Config config = {HIGH,
                                 {true}};

} // namespace preset

} // namespace cfg
} // namespace pt
