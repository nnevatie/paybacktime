#include "render_stats.h"

#include <sstream>
#include <GL/glew.h>

#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg_gl.h"

#include "common.h"

namespace hc
{
namespace gl
{

}

RenderStats::RenderStats() : frameTime(10), vertexCount(0), triangleCount(0)
{
    vg = nvgCreateGL2(0);
    nvgCreateFont(vg, "dejavu-sans", "data/deja_vu_sans.ttf");
}

RenderStats::~RenderStats()
{
    nvgDeleteGL2(vg);
}

void RenderStats::accumulate(float frameTime, int vertexCount, int triangleCount)
{
    this->frameTime.push(frameTime);
    this->vertexCount   = vertexCount;
    this->triangleCount = triangleCount;
}

void RenderStats::render(int width, int height, float scale)
{
    glPolygonMode(GL_FRONT, GL_FILL);
    nvgBeginFrame(vg, width, height, scale);
    nvgFontSize(vg, 16.0f);
    nvgFontFace(vg, "dejavu-sans");
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));

    float timeMs = frameTime.mean() * 0.001f;
    nvgText(vg, 10, 20, str(std::stringstream()
                            << "Time: " << timeMs << " ms").c_str(), 0);
    nvgText(vg, 140, 20, str(std::stringstream()
                            << "FPS: " << (1000.f / timeMs) << " ms").c_str(), 0);

    nvgText(vg, 10, 40, str(std::stringstream()
                            << "Vertices: " << vertexCount).c_str(), 0);
    nvgText(vg, 140, 40, str(std::stringstream()
                            << "Triangles: " << triangleCount).c_str(), 0);

    nvgEndFrame(vg);
}

} // namespace hc
