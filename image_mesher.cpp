#include "image_mesher.h"

#include <array>

#include <glm/ext.hpp>

#include "clock.h"
#include "log.h"

namespace hc
{
namespace ImageMesher
{

namespace
{

struct Heightfield
{
    Heightfield(int width, int height) :
        width(width), height(height),
        interval(1.f),
        values(width * height, 0.f)
    {
    }

    Heightfield(const Image& image, float interval = 1.f) :
        width(image.rect().w / interval), height(image.rect().h / interval),
        interval(interval),
        values(width * height)
    {
        const Rect<int>             rect   = image.rect();
        const uint8_t* __restrict__ bits   = image.bits();
        const int                   stride = image.stride();
        const float                 depth  = std::min(rect.w, rect.h) / 255.f;

        for (int sy = 0; sy < height; ++sy)
        {
            const float                  fy = rect.y + sy * interval;
            const int                     y = int(fy);
            const uint8_t* __restrict__ row = bits + y * stride;

            for (int sx = 0; sx < width; ++sx)
            {
                const int fx            = int(rect.x + sx * interval);
                const int x             = int(fx);
                const int p             = row[x];
                values[sy * width + sx] = p * depth;
            }
        }
    }

    bool operator()(int x, int y, int z) const
    {
        return values.at(y * width + x) > z;
    }

    int   width, height;
    float interval;

    std::vector<float> values;
};

Geometry meshHeightfield(const Heightfield& hfield)
{
    Geometry geometry;
    geometry.vertices.reserve(hfield.values.size() * 8);
    geometry.indices.reserve(hfield.values.size() * 12);

    const float* __restrict__ data = hfield.values.data();
    const float               size = hfield.interval;

    for (int y = 0; y < hfield.height; ++y)
        for (int x = 0; x < hfield.width; ++x)
        {
            const float h      = data[y * hfield.width + x];
            const glm::vec3 c0 = {x * size  + 0, y * size  + 0, 0.f};
            const glm::vec3 c1 = {x * size  + size , y * size  + size , h};
            const int ib       = geometry.vertices.size();

            const glm::vec3 vertices[] =
            {
                // Front
                {c0.x, c0.y, c0.z},
                {c1.x, c0.y, c0.z},
                {c1.x, c1.y, c0.z},
                {c0.x, c1.y, c0.z},
                // Back
                {c0.x, c0.y, c1.z},
                {c1.x, c0.y, c1.z},
                {c1.x, c1.y, c1.z},
                {c0.x, c1.y, c1.z}

            };
            geometry.vertices.insert(geometry.vertices.end(),
                                     std::begin(vertices), std::end(vertices));

            typedef uint16_t index;
            const uint16_t indices[] =
            {
                // Front
                index(ib + 0), index(ib + 1), index(ib + 2),
                index(ib + 2), index(ib + 3), index(ib + 0),
                // Top
                index(ib + 3), index(ib + 2), index(ib + 6),
                index(ib + 6), index(ib + 7), index(ib + 3),
                // Back
                index(ib + 7), index(ib + 6), index(ib + 5),
                index(ib + 5), index(ib + 4), index(ib + 7),
                // Bottom
                index(ib + 4), index(ib + 5), index(ib + 1),
                index(ib + 1), index(ib + 0), index(ib + 4),
                // Left
                index(ib + 4), index(ib + 0), index(ib + 3),
                index(ib + 3), index(ib + 7), index(ib + 4),
                // Right
                index(ib + 1), index(ib + 5), index(ib + 6),
                index(ib + 6), index(ib + 2), index(ib + 1)
            };
            geometry.indices.insert(geometry.indices.end(),
                                    std::begin(indices), std::end(indices));
        }
    return geometry;
}

template <typename V>
Geometry meshGreedy(const V& vol, const std::array<int, 3>& dims)
{
    Geometry geometry;
    for (int d = 0; d < 3; ++d)
    {
        const int u = (d + 1) % 3;
        const int v = (d + 2) % 3;

        int x[3] = {0};
        int q[3] = {0};
        int mask[dims[u] * dims[v]];
        int n;

        q[d] = 1;
        for (x[d] = -1; x[d] < dims[d];)
        {
            // Determine mask
            n = 0;
            for (x[v] = 0; x[v] < dims[v]; ++x[v])
                for (x[u] = 0; x[u] < dims[u]; ++x[u])
                {
                    mask[n++] = (0 <= x[d] ?
                                  vol(x[0], x[1], x[2]) : 0) !=
                                 (x[d] < dims[d] - 1 ?
                                  vol(x[0] + q[0], x[1] + q[1], x[2] + q[2]) : 0);
                }

            ++x[d];
            n = 0;

            for (int j = 0; j < dims[v]; ++j)
                for (int i = 0; i < dims[u];)
                {
                    if (mask[n])
                    {
                        // Dimensions
                        int w = 0, h = 0;
                        // Compute width
                        for (w = 1; mask[n + w] && i + w < dims[u]; ++w)
                        {}
                        // Compute height
                        for (h = 1; j + h < dims[v]; ++h)
                            for (int k = 0; k < w; ++k)
                                if (!mask[n + k + h * dims[u]])
                                    goto dims_done;
                        dims_done:

                        x[u] = i;
                        x[v] = j;

                        // Emit quad
                        int du[3] = {0};
                        int dv[3] = {0};
                        du[u]     = w;
                        dv[v]     = h;

                        const int ib = geometry.vertices.size();
                        const glm::vec3 vertices[] =
                        {
                            {x[0], x[1], x[2]},
                            {x[0] + du[0], x[1] + du[1], x[2] + du[2]},
                            {x[0] + du[0] + dv[0], x[1] + du[1] + dv[1],
                             x[2] + du[2] + dv[2]},
                            {x[0] + dv[0], x[1] + dv[1], x[2] + dv[2]},
                        };
                        geometry.vertices.insert(geometry.vertices.end(),
                                                 std::begin(vertices),
                                                 std::end(vertices));

                        typedef uint16_t index;
                        const uint16_t indices[] = {index(ib + 0),
                                                    index(ib + 1),
                                                    index(ib + 2),
                                                    index(ib + 2),
                                                    index(ib + 3),
                                                    index(ib + 0)};

                        geometry.indices.insert(geometry.indices.end(),
                                                std::begin(indices),
                                                std::end(indices));

                        // Clear mask
                        for (int l = 0; l < h; ++l)
                            for (int k = 0; k < w; ++k)
                                mask[n + k + l * dims[u]] = 0;

                        // Increment counters
                        i += w;
                        n += w;
                    }
                    else
                    {
                        ++i;
                        ++n;
                    }
                }
        }
    }
    return geometry;
}

}

Geometry geometry(const Image& image, float interval)
{
    HCTIME("emit geom");
    Heightfield hfield(image, interval);
    //return meshHeightfield(hfield);
    return meshGreedy(hfield, {hfield.width, hfield.height, hfield.width});
}

} // namespace ImageMesher
} // namespace

