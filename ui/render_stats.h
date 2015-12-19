#pragma once

#include "common/clock.h"
#include "common/statistics.h"

struct NVGcontext;

namespace hc
{
namespace ui
{

struct RenderStats
{
    RenderStats();
    ~RenderStats();

    void accumulate(const Duration& frameTime,
                    int vertexCount, int triangleCount);
    void render();

private:
    NVGcontext* vg;

    float accumTime, meanTimeMs;
    MovingAvg<float> frameTimes;

    int vertexCount, triangleCount;
};

} // namespace ui
} // namespace hc
