#include "render_stats.h"

#include <sstream>
#include <iomanip>

#include "platform/gl.h"
#include <nanovg.h>

#include "common/common.h"
#include "geom/mesh.h"

namespace pt
{
namespace ui
{

const float UPDATE_INTERVAL_US = 1000.f * 100;

struct RenderStats::Data
{
    explicit Data(NVGcontext* vg) :
        vg(vg), accumTime(0), meanTimeMs(0),
        frameTimes(10), vertexCount(0), triangleCount(0)
    {
        nvgCreateFont(vg, "conradi", "data/conradi_square.ttf");
    }
    ~Data()
    {
        // TODO: Free font?
    }

    NVGcontext* vg;

    float accumTime, meanTimeMs;
    MovingAvg<float> frameTimes;

    int vertexCount, triangleCount;
};

RenderStats::RenderStats(NVGcontext* vg) :
    d(std::make_shared<Data>(vg))
{
}

void RenderStats::accumulate(const pt::Duration& frameTime,
                             int vertexCount, int triangleCount)
{
    const float timeUs = boost::chrono::duration
                         <float, boost::micro>(frameTime).count();

    d->frameTimes.push(timeUs);
    d->vertexCount   = vertexCount;
    d->triangleCount = triangleCount;

    d->accumTime += timeUs;
    if (d->accumTime >= UPDATE_INTERVAL_US || !d->meanTimeMs)
    {
        d->meanTimeMs = d->frameTimes.mean() * 0.001f;
        d->accumTime  = std::fmod(d->accumTime, UPDATE_INTERVAL_US);
    }
}

RenderStats& RenderStats::operator()(const glm::ivec3& sceneSize)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    nvgBeginFrame(d->vg, viewport[2], viewport[3], 1.f);
    nvgFontSize(d->vg, 22.0f);
    nvgFontFace(d->vg, "conradi");
    nvgFillColor(d->vg, nvgRGBA(255, 255, 255, 255));

    const float timeMs = d->meanTimeMs;

    nvgText(d->vg, 10, 20, str(std::stringstream()
                               << std::fixed << std::setprecision(1)
                               << "Time: " << timeMs << " ms").c_str(), 0);
    nvgText(d->vg, 140, 20, str(std::stringstream()
                               << std::fixed << std::setprecision(1)
                               << "FPS: " << (1000.f / timeMs)).c_str(), 0);
    nvgText(d->vg, 250, 20, str(std::stringstream()
                               << "Scene: "
                               << sceneSize.x << "x"
                               << sceneSize.y << "x"
                               << sceneSize.z).c_str(), 0);

#if 0
    nvgText(vg, 10, 40, str(std::stringstream()
                            << "Vertices: " << vertexCount).c_str(), 0);
    nvgText(vg, 140, 40, str(std::stringstream()
                            << "Triangles: " << triangleCount).c_str(), 0);

    int geomKb = 0.001 * (vertexCount * sizeof(Mesh_P_N_UV::Vertex) +
                          triangleCount * 3 * sizeof(Mesh_P_N_UV::Index));

    nvgText(vg, 10, 60, str(std::stringstream()
                            << "Geometry: ~" << geomKb << " KB").c_str(), 0);
#endif
    nvgEndFrame(d->vg);
    return *this;
}

} // namespace ui
} // namespace pt
