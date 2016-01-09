#pragma once

#include <memory>

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

    void accumulate(const Duration& frameTime,
                    int vertexCount, int triangleCount);

    RenderStats& operator()();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
