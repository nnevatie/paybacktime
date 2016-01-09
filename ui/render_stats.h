#pragma once

#include "platform/clock.h"
#include "common/statistics.h"

struct NVGcontext;

namespace pt
{
namespace ui
{

struct RenderStats
{
    RenderStats(NVGcontext* vg);
    ~RenderStats();

    void accumulate(const Duration& frameTime,
                    int vertexCount, int triangleCount);

    RenderStats& operator()();

private:
    NVGcontext* vg;

    float accumTime, meanTimeMs;
    MovingAvg<float> frameTimes;

    int vertexCount, triangleCount;
};

} // namespace ui
} // namespace pt
