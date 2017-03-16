#pragma once

#include <memory>

#include <glm/vec3.hpp>

#include "platform/clock.h"
#include "common/statistics.h"
#include "gl/gpu_clock.h"

struct NVGcontext;

namespace pt
{
namespace ui
{

struct RenderStats
{
    using Time = TimeTree<GpuClock>;

    RenderStats(NVGcontext* vg);

    void accumulate(const Time& frameTime);

    RenderStats& operator()(float fps, const glm::ivec3& sceneSize);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
