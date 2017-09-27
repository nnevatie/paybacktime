#pragma once

#include <vector>

#include <boost/algorithm/clamp.hpp>

#include <glm/vec3.hpp>

#include "img/image.h"
#include "img/image_cube.h"

namespace pt
{

struct Heightfield
{
    Heightfield() : width(0), height(0), depth(0)
    {}

    Heightfield(int width, int height) :
        width(width), height(height), depth(0),
        values(width * height, 0.f)
    {}

    Heightfield(const Image& image, int depth) :
        width(image.size().w), height(image.size().h),
        depth(depth),
        values(width * height)
    {
        const uint8_t* __restrict__ bits   = image.bits();
        const int                   stride = image.stride();

        for (int sy = 0; sy < height; ++sy)
        {
            const float                  fy = sy;
            const int                     y = int(fy);
            const uint8_t* __restrict__ row = bits + y * stride;

            for (int sx = 0; sx < width; ++sx)
            {
                const int fx            = int(sx);
                const int x             = int(fx);
                const int p             = row[x];
                const float v           = std::pow(float(p) / 255.f, 0.45f);
                values[sy * width + sx] = v * depth;
            }
        }
    }

    inline int f(int x, int y) const
    {
        using namespace boost::algorithm;
        x = clamp(x, 0, width  - 1);
        y = clamp(y, 0, height - 1);
        return values[y * width + x];
    }

    inline int g(int x, int y, int z) const
    {
        const int vc = f(x, y);
        if (vc - z > 1) return 0;

        const int v[8] =
        {
            vc - f(x - 1, y - 1),
            vc - f(x + 0, y - 1),
            vc - f(x + 1, y - 1),
            vc - f(x - 1, y + 0),
            vc - f(x + 1, y + 0),
            vc - f(x - 1, y + 1),
            vc - f(x + 0, y + 1),
            vc - f(x + 1, y + 1)
        };

        const float e  = 0.5f * depth;
        auto collapsed = [&](int i) {return v[i] > 0 && v[i] < e;};

        int gradient = 0;
        if (collapsed(0) || collapsed(1) || collapsed(3))
            gradient |= 0x01;
        if (collapsed(1) || collapsed(2) || collapsed(4))
            gradient |= 0x02;
        if (collapsed(3) || collapsed(5) || collapsed(6))
            gradient |= 0x04;
        if (collapsed(4) || collapsed(6) || collapsed(7))
            gradient |= 0x08;

        return gradient;
    }

    inline bool operator()(int x, int y, int z) const
    {
        return f(x, y) > z;
    }

    int width, height, depth;
    std::vector<int> values;
};

struct Cubefield
{
    Cubefield(const ImageCube& imageCube) :
        width(imageCube.width()),
        height(imageCube.height()),
        depth(imageCube.depth())
    {
        const int depths[6] = {depth, depth, width, width, height, height};
        const int sides[]   = {0, 1, 3, 2, 4, 5}; // Left & right swapped
        for (int i = 0; i < int(imageCube.sides.size()); ++i)
            hfields[i] = Heightfield(imageCube.sides[sides[i]],
                                     depths[sides[i]]);
    }

    inline int g(int x, int y, int z) const
    {
        const int c[6][3] =
        {
            {x, y,              z},
            {x, y,  depth - z - 1},
            {z, y,              x},
            {z, y,  width - x - 1},
            {x, z,              y},
            {x, z, height - y - 1}
        };
        int gradient = 0;
        for (int i = 0; i < 6; ++i)
            gradient |= hfields[i].g(c[i][0], c[i][1], c[i][2]) << (i << 2);

        return gradient;
    }

    inline bool operator()(int x, int y, int z) const
    {
        return hfields[0](x, y,              z) &&
               hfields[1](x, y,  depth - z - 1) &&
               hfields[2](z, y,              x) &&
               hfields[3](z, y,  width - x - 1) &&
               hfields[4](x, z,              y) &&
               hfields[5](x, z, height - y - 1);
    }

    inline glm::vec3 size() const
    {
        return {width, height, depth};
    }

    // Front, back, left, right, top, bottom
    Heightfield hfields[6];
    int width, height, depth;
};

} // namespace pt
