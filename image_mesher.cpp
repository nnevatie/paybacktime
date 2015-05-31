#include "image_mesher.h"

#include <array>

#include <glm/ext.hpp>

#include "sn_mesher.h"

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
    Heightfield() : width(0), height(0), depth(0), interval(0)
    {
    }

    Heightfield(int width, int height) :
        width(width), height(height), depth(0),
        interval(1.f),
        values(width * height, 0.f)
    {
    }

    Heightfield(const Image& image, int depth, float interval = 1.f) :
        width(image.rect().w / interval), height(image.rect().h / interval),
        depth(depth),
        interval(interval),
        values(width * height)
    {
        const Rect<int>             rect   = image.rect();
        const uint8_t* __restrict__ bits   = image.bits();
        const int                   stride = image.stride();

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
                const float v           = std::pow(float(p) / 255.f, 0.45f);
                values[sy * width + sx] = v * depth;
            }
        }
    }

    inline int f(int x, int y) const
    {
        return x >= 0 && x < width && y >= 0 && y < height ?
               values.at(y * width + x) : 0;
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

        const float e = depth / 2.f;
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

    int   width, height, depth;
    float interval;

    std::vector<int> values;
};

struct Cubefield
{
    Cubefield(const ImageCube& imageCube, float interval = 1.f) :
        width(imageCube.width()   / interval),
        height(imageCube.height() / interval),
        depth(imageCube.depth()   / interval),
        interval(interval)
    {
        const int depths[6] = {depth, depth, width, width, height, height};
        for (int i = 0; i < 6; ++i)
            hfields[i] = Heightfield(imageCube.sides[i], depths[i], interval);
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
            gradient |= hfields[i].g(c[i][0], c[i][1], c[i][2]) << (4 * i);

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

    // Front, back, left, right, top, bottom
    Heightfield hfields[6];

    int   width, height, depth;
    float interval;
};

void emitBox(Geometry* g, const Box& box)
{
    const uint16_t ib = g->vertices.size();
    g->vertices.insert(g->vertices.end(),
                       box.begin(),
                       box.end());

    typedef uint16_t index;
    const uint16_t indices[] = {// Front
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
                                index(ib + 6), index(ib + 2), index(ib + 1)};

    g->indices.insert(g->indices.end(),
                      std::begin(indices),
                      std::end(indices));
}

template <typename V>
bool visible(const V& vol, int x, int y, int z)
{
    return vol(x + 0, y + 0, z + 0);
    return  vol(x + 0, y + 0, z + 0) &&
            (
            !vol(x - 1, y + 0, z + 0) ||
            !vol(x + 1, y + 0, z + 0) ||
            !vol(x + 0, y - 1, z + 0) ||
            !vol(x + 0, y + 1, z + 0) ||
            !vol(x + 0, y + 0, z - 1) ||
            !vol(x + 0, y + 0, z + 1)
            );
}

Box box(const glm::vec3& v0, const glm::vec3& v1)
{
    return
    {{
        // Front
        {v0.x, v0.y, v1.z},
        {v1.x, v0.y, v1.z},
        {v1.x, v1.y, v1.z},
        {v0.x, v1.y, v1.z},
        // Back
        {v0.x, v0.y, v0.z},
        {v1.x, v0.y, v0.z},
        {v1.x, v1.y, v0.z},
        {v0.x, v1.y, v0.z}
    }};
}

template <typename V>
Box box(const V& vol, int x, int y, int z)
{
    const int  g = vol.g(x, y, z);
    const auto d = [=](int c, int s) {return bool(g & (c << s));};
    const int c[8][3] =
    {
        // Front
        {x +  d(0x02, 12), y +  d(0x04, 20), z + !d(0x01, 0)},
        {x + !d(0x02,  8), y +  d(0x08, 20), z + !d(0x02, 0)},
        {x + !d(0x08,  8), y + !d(0x08, 16), z + !d(0x08, 0)},
        {x +  d(0x08, 12), y + !d(0x04, 16), z + !d(0x04, 0)},
        // Back
        {x +  d(0x01, 12), y +  d(0x01, 20), z +  d(0x01, 4)},
        {x + !d(0x01,  8), y +  d(0x02, 20), z +  d(0x02, 4)},
        {x + !d(0x04,  8), y + !d(0x02, 16), z +  d(0x08, 4)},
        {x +  d(0x04, 12), y + !d(0x01, 16), z +  d(0x04, 4)}
    };
    const float s = vol.interval;
    return
    {
        glm::vec3(c[0][0], c[0][1], c[0][2]) * s,
        glm::vec3(c[1][0], c[1][1], c[1][2]) * s,
        glm::vec3(c[2][0], c[2][1], c[2][2]) * s,
        glm::vec3(c[3][0], c[3][1], c[3][2]) * s,
        glm::vec3(c[4][0], c[4][1], c[4][2]) * s,
        glm::vec3(c[5][0], c[5][1], c[5][2]) * s,
        glm::vec3(c[6][0], c[6][1], c[6][2]) * s,
        glm::vec3(c[7][0], c[7][1], c[7][2]) * s,
    };
}

template <typename V>
Geometry meshCubes(const V& vol)
{
    const std::array<int, 3>& dims = {vol.width, vol.height, vol.depth};

    Geometry geometry;
    geometry.vertices.reserve(dims[0] * dims[1] * dims[2] * 8);
    geometry.indices.reserve(dims[0] * dims[1] * dims[2] * 36);

    for (int z = 0; z < dims[2]; ++z)
        for (int y = 0; y < dims[1]; ++y)
            for (int x = 0; x < dims[0]; ++x)
                if (visible(vol, x, y, z))
                    emitBox(&geometry, box(vol, x, y, z));

    return geometry;
}

template <typename V>
Geometry meshGreedy(const V& vol)
{
    const std::array<int, 3>& dims = {vol.width, vol.height, vol.depth};
    const float                  s =  vol.interval;

    const int allocEstimate = (dims[0] / 4) * (dims[1] / 4) * (dims[2] / 4);

    Geometry geometry;
    geometry.vertices.reserve(allocEstimate);
    geometry.indices.reserve(allocEstimate);

    for (int d = 0; d < 3; ++d)
    {
        const int u = (d + 1) % 3;
        const int v = (d + 2) % 3;

        int x[3] = {0};
        int q[3] = {0};
        q[d]     = 1;

        char mask[dims[u] * dims[v]];

        for (x[d] = -1; x[d] < dims[d];)
        {
            // Determine mask
            int n = 0;
            for (x[v] = 0; x[v] < dims[v]; ++x[v])
                for (x[u] = 0; x[u] < dims[u]; ++x[u])
                {
                    const int v0 = vol(x[0], x[1], x[2]);
                    const int v1 = vol(x[0] + q[0], x[1] + q[1], x[2] + q[2]);
                    mask[n++] = v0 - v1;
                }

            ++x[d];
            n = 0;

            for (int j = 0; j < dims[v]; ++j)
                for (int i = 0; i < dims[u];)
                {
                    const int m = mask[n];
                    if (m)
                    {
                        // Dimensions
                        int w = 0, h = 0;
                        // Compute width
                        for (w = 1; m == mask[n + w] && i + w < dims[u]; ++w)
                        {}
                        // Compute height
                        for (h = 1; j + h < dims[v]; ++h)
                            for (int k = 0; k < w; ++k)
                                if (m != mask[n + k + h * dims[u]])
                                    goto dims_done;
                        dims_done:

                        x[u] = i;
                        x[v] = j;

                        // Emit quad
                        int du[3] = {0};
                        int dv[3] = {0};

                        if (m > 0)
                        {
                            du[u] = w;
                            dv[v] = h;
                        }
                        else
                        {
                            dv[u] = w;
                            du[v] = h;
                        }

                        typedef glm::vec3 v3;
                        const Geometry::Vertex vertices[] =
                        {
                            s * v3(x[0],
                                   x[1],
                                   x[2]),

                            s * v3(x[0] + du[0],
                                   x[1] + du[1],
                                   x[2] + du[2]),

                            s * v3(x[0] + du[0] + dv[0],
                                   x[1] + du[1] + dv[1],
                                   x[2] + du[2] + dv[2]),

                            s * v3(x[0] + dv[0],
                                   x[1] + dv[1],
                                   x[2] + dv[2])
                        };

                        const int ib = geometry.vertices.size();
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

} // namespace

Geometry geometry(const Image& image, float interval)
{
    HCTIME("image geom");
    const Heightfield hfield(image, std::min(image.rect().w,
                                             image.rect().h), interval);

    //return meshGreedy(hfield);
    return meshCubes(hfield);
}

Geometry geometry(const ImageCube& imageCube, float interval)
{
    HCTIME("cube geom");
    const Cubefield cfield(imageCube, interval);

    //return meshGreedy(cfield);
    return meshCubes(cfield);
}

} // namespace ImageMesher
} // namespace hc

