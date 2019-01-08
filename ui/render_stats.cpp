#include "render_stats.h"

#include <sstream>
#include <iomanip>
#include <unordered_map>

#include <nanovg.h>

#include "common/common.h"
#include "geom/mesh.h"

namespace pt
{
namespace ui
{

constexpr int   MOVING_AVG_LEN     = 10;
constexpr float UPDATE_INTERVAL_US = 1000.f * 100;

struct RenderStats::Data
{
    explicit Data(NVGcontext* vg) :
        vg(vg)
    {
        nvgCreateFont(vg, "sans", "data/play_regular.ttf");
    }

    NVGcontext* vg;
    std::unordered_map<std::string, MovingAvg<float>> times;
};

RenderStats::RenderStats(NVGcontext* vg) :
    d(std::make_shared<Data>(vg))
{
}

void RenderStats::accumulate(const Time& frameTime)
{
    using Milli = std::chrono::duration<float, std::milli>;
    for (const auto& t : frameTime.map())
    {
        auto it = d->times.find(t.first);
        if (it != d->times.end())
            it->second.push(Milli(t.second.duration()).count());
        else
            d->times.insert({t.first, MovingAvg<float>(MOVING_AVG_LEN)});
    }
}

RenderStats& RenderStats::operator()(float fps, const glm::ivec3& sceneSize)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    nvgBeginFrame(d->vg, viewport[2], viewport[3], 1.f);
    nvgFontSize(d->vg, 18.0f);
    nvgFontFace(d->vg, "sans");
    nvgFillColor(d->vg, nvgRGBA(255, 255, 255, 255));

    nvgText(d->vg, 10, 20, str(std::stringstream()
                               << std::fixed << std::setprecision(1)
                               << "FPS: " << fps).c_str(), 0);
    nvgText(d->vg, 140, 20, str(std::stringstream()
                               << "Scene: "
                               << sceneSize.x << "x"
                               << sceneSize.y << "x"
                               << sceneSize.z).c_str(), 0);

    std::vector<std::pair<std::string, float>> times;
    for (const auto& t : d->times)
    {
        const auto time = t.second.mean();
        int index = 0;
        for (; index < int(times.size()); ++index)
            if (time > times.at(index).second)
                break;

        times.insert(times.begin() + index, std::make_pair(t.first, time));
    }

    if (d->times.find("total") != d->times.end())
    {
        const auto timeTotal = d->times.at("total").mean();

        int row = 0;
        nvgFillColor(d->vg, nvgRGBA(150, 180, 200, 255));
        for (const auto& t : times)
        {
            const auto time = t.second;
            const auto f    = time / timeTotal;
            nvgText(d->vg, 10, 40 + row * 20,
                    str(std::stringstream()
                        << t.first << ":").c_str(), 0);
            nvgText(d->vg, 140, 40 + row * 20,
                    str(std::stringstream()
                        << std::fixed << std::setprecision(3)
                        << time << " ms, " << (f * 100.f) << "%").c_str(), 0);
            ++row;
        }
    }
    nvgEndFrame(d->vg);
    return *this;
}

} // namespace ui
} // namespace pt
