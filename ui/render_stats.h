#pragma once

#include <memory>

#include <glm/vec3.hpp>

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

    RenderStats& operator()(const glm::ivec3& sceneSize);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
