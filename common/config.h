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

    Output(float scale) : scale(scale)
    {}

    glm::vec2 renderSize() const
    {
        return scale * size;
    }
};

struct Scattering
{
    float scale;
    int   samples;
};

struct Ssao
{
    float scale;
    int   samples;
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

static const Video
    ULTRA = {{1.00f}, {1.00f, 32}, {1.00f}, {1.00f}, {1.00f, 20}},
    HIGH  = {{1.00f}, {1.00f, 24}, {0.50f}, {1.00f}, {0.50f, 15}},
    LOW   = {{1.00f}, {0.50f, 16}, {0.25f}, {0.50f}, {0.25f, 10}};

static const Config config = {HIGH, {false}};

} // namespace preset

} // namespace cfg
} // namespace pt
