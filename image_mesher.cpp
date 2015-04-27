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
        return values.at(y * width + x) >= z;
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
Geometry meshGreedy(const V& volume, const std::array<int, 3>& dims)
{
    Geometry geometry;
    return geometry;
}

}

Geometry geometry(const Image& image, float interval)
{
    HCTIME("emit geom");
    Heightfield hfield(image, interval);
    return meshHeightfield(hfield);
    //return meshGreedy(hfield, {hfield.width, hfield.height, hfield.width});
}

} // namespace ImageMesher
} // namespace

