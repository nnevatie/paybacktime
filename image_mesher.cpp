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
    int c[8][3] =
    {
        // Front
        {x + 0, y + 0, z + 1},
        {x + 1, y + 0, z + 1},
        {x + 1, y + 1, z + 1},
        {x + 0, y + 1, z + 1},
        // Back
        {x + 0, y + 0, z + 0},
        {x + 1, y + 0, z + 0},
        {x + 1, y + 1, z + 0},
        {x + 0, y + 1, z + 0}
    };

    const float s = vol.interval;
    glm::vec3 v[8];
    for (int i = 0; i < 8; ++i)
        v[i] = glm::vec3(c[i][0], c[i][1], c[i][2]) * s;

    Box box;
    std::copy(std::begin(v), std::end(v), box.begin());
    return box;
}

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
        const float                 scale  = depth / 255.f;

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
                values[sy * width + sx] = p * scale;
            }
        }
    }

    inline float value(int x, int y) const
    {
        return x >= 0 && x < width && y >= 0 && y < height ?
               values.at(y * width + x) : 0;
    }

    inline bool operator()(int x, int y, int z) const
    {
        return value(x, y) > z;
    }

    int   width, height, depth;
    float interval;

    std::vector<float> values;
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

    bool operator()(int x, int y, int z) const
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

Geometry meshHeightfield(const Heightfield& hfield)
{
    Geometry geometry;
    geometry.vertices.reserve(hfield.values.size() * 8);
    geometry.indices.reserve(hfield.values.size() * 12);

    const float* __restrict__ data = hfield.values.data();
    const float              scale = hfield.interval;

    for (int y = 0; y < hfield.height; ++y)
        for (int x = 0; x < hfield.width; ++x)
        {
            const float h      = data[y * hfield.width + x];
            const glm::vec3 c0 = {x * scale + 0,     y * scale + 0,     0.f};
            const glm::vec3 c1 = {x * scale + scale, y * scale + scale, h};
            const int ib       = geometry.vertices.size();

            const Geometry::Vertex vertices[] =
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

