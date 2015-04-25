#pragma once

#include "statistics.h"

struct NVGcontext;

namespace hc
{

struct RenderStats
{
    RenderStats();
    ~RenderStats();

    void accumulate(float frameTime, int vertexCount, int triangleCount);
    void render(int width, int height, float scale = 1.f);

private:
    NVGcontext* vg;

    MovingAvg<int64_t> frameTime;
    int vertexCount, triangleCount;
};

} // namespace hc
