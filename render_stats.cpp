#include "render_stats.h"

#include <sstream>
#include <iomanip>

#include <glad/glad.h>

#include <glad/glad.h>

#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>

#include "mesh.h"
#include "common.h"

namespace hc
{
const float UPDATE_INTERVAL_US = 1000.f * 200;

RenderStats::RenderStats() :
    accumTime(0), meanTimeMs(0),
    frameTimes(100), vertexCount(0), triangleCount(0)
{
    vg = nvgCreateGL3(0);
    nvgCreateFont(vg, "dejavu-sans", "data/deja_vu_sans.ttf");
}

RenderStats::~RenderStats()
{
    nvgDeleteGL3(vg);
}

void RenderStats::accumulate(float frameTime, int vertexCount, int triangleCount)
{
    this->frameTimes.push(frameTime);
    this->vertexCount   = vertexCount;
    this->triangleCount = triangleCount;

    accumTime += frameTime;
    if (accumTime >= UPDATE_INTERVAL_US || !meanTimeMs)
    {
        meanTimeMs = frameTimes.mean() * 0.001f;
        accumTime  = std::fmod(accumTime, UPDATE_INTERVAL_US);
    }
}

void RenderStats::render()
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    nvgBeginFrame(vg, viewport[2], viewport[3], 1.f);
    nvgFontSize(vg, 16.0f);
    nvgFontFace(vg, "dejavu-sans");
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));

    const float timeMs = meanTimeMs;

    nvgText(vg, 10, 20, str(std::stringstream()
                            << std::fixed << std::setprecision(1)
                            << "Time: " << timeMs << " ms").c_str(), 0);
    nvgText(vg, 140, 20, str(std::stringstream()
                            << std::fixed << std::setprecision(1)
                            << "FPS: " << (1000.f / timeMs)).c_str(), 0);

    nvgText(vg, 10, 40, str(std::stringstream()
                            << "Vertices: " << vertexCount).c_str(), 0);
    nvgText(vg, 140, 40, str(std::stringstream()
                            << "Triangles: " << triangleCount).c_str(), 0);

    int geomKb = 0.001 * (vertexCount * sizeof(Mesh_P_N_UV::Vertex) +
                          triangleCount * 3 * sizeof(Mesh_P_N_UV::Index));

    nvgText(vg, 10, 60, str(std::stringstream()
                            << "Geometry: ~" << geomKb << " KB").c_str(), 0);
    nvgEndFrame(vg);
}

} // namespace hc
