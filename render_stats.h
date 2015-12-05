#pragma once

#include "common/statistics.h"

struct NVGcontext;

namespace hc
{

struct RenderStats
{
    RenderStats();
    ~RenderStats();

    void accumulate(float frameTime, int vertexCount, int triangleCount);
    void render();

private:
    NVGcontext* vg;

    float accumTime, meanTimeMs;
    MovingAvg<int64_t> frameTimes;

    int vertexCount, triangleCount;
};

} // namespace hc
