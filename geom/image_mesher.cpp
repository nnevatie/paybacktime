#include "image_mesher.h"

#include <array>

#include <glm/ext.hpp>

#include "platform/clock.h"
#include "common/log.h"

#include "volume.h"
#include "rect.h"

#include "mesher_common.h"
#include "sn_mesher.h"

namespace pt
{
namespace ImageMesher
{

namespace
{

void emitBoxFace(Mesh_P_N_T_UV* g, float scale, int axis, int cc[8][3],
                 const glm::vec3& p, const glm::vec3& s, const glm::vec3& d,
                 const RectCube<float>& uvCube)
{
    using V = glm::vec3;

    const int i[6][4] =
    {
        {3, 7, 4, 0},
        {6, 2, 1, 5},
        {4, 5, 1, 0},
        {7, 3, 2, 6},
        {7, 6, 5, 4},
        {0, 1, 2, 3}
    };
    const V va = scale * V(p.x + (cc[i[axis][0]][0] * s.x),
                           p.y + (cc[i[axis][0]][1] * s.y),
                           p.z + (cc[i[axis][0]][2] * s.z));
    const V vb = scale * V(p.x + (cc[i[axis][1]][0] * s.x),
                           p.y + (cc[i[axis][1]][1] * s.y),
                           p.z + (cc[i[axis][1]][2] * s.z));
    const V vc = scale * V(p.x + (cc[i[axis][2]][0] * s.x),
                           p.y + (cc[i[axis][2]][1] * s.y),
                           p.z + (cc[i[axis][2]][2] * s.z));
    const V vd = scale * V(p.x + (cc[i[axis][3]][0] * s.x),
                           p.y + (cc[i[axis][3]][1] * s.y),
                           p.z + (cc[i[axis][3]][2] * s.z));

    emitQuad(g, axis, p, s, d, {va, vb, vc, vd}, uvCube);
}

void collapseConstants(int cc[8][3], int g)
{
    const auto d = [=](int c, int s) {return bool(g & (c << s));};
    int c[8][3] =
    {
        // Front
        { d(0x02, 12),  d(0x04, 20), !d(0x01, 0)},
        {!d(0x02,  8),  d(0x08, 20), !d(0x02, 0)},
        {!d(0x08,  8), !d(0x08, 16), !d(0x08, 0)},
        { d(0x08, 12), !d(0x04, 16), !d(0x04, 0)},
        // Back
        { d(0x01, 12),  d(0x01, 20),  d(0x01, 4)},
        {!d(0x01,  8),  d(0x02, 20),  d(0x02, 4)},
        {!d(0x04,  8), !d(0x02, 16),  d(0x08, 4)},
        { d(0x04, 12), !d(0x01, 16),  d(0x04, 4)}
    };
    std::copy(&c[0][0], &c[0][0] + 8 * 3, &cc[0][0]);
}

template <typename V>
Mesh_P_N_T_UV meshGreedy(const V& vol,
                         const RectCube<float>& uvCube,
                         float scale = 1.f,
                         bool greedy = true)
{
    struct Cell
    {
        int v, g;
    };

    struct Mask
    {
        int d, g0, g1;

        bool operator==(const Mask& mask) const
        {
            return d == mask.d && g0 == mask.g0 && g1 == mask.g1;
        }
        bool operator!=(const Mask& mask) const
        {
            return !operator==(mask);
        }
    };

    // Dimensions
    const int dims[3] = {vol.width, vol.height, vol.depth};

    // Inside-check
    const auto in = [&](const int c[3], const int d[3])
    {return c[0] >= 0    && c[1] >= 0    && c[2] >= 0 &&
            c[0] <  d[0] && c[1] <  d[1] && c[2] <  d[2];};

    Cell cells[dims[0]][dims[1]][dims[2]];
    for (int z = 0; z < dims[2]; ++z)
        for (int y = 0; y < dims[1]; ++y)
            for (int x = 0; x < dims[0]; ++x)
            {
                const int v = vol(x, y, z);
                cells[x][y][z].v = v;
                cells[x][y][z].g = v ? vol.g(x, y, z) : 0;
            }

    Mesh_P_N_T_UV mesh;
    const int reserveSize = (dims[0] / 4) * (dims[1] / 4) * (dims[2] / 4);
    mesh.vertices.reserve(reserveSize);
    mesh.indices.reserve(reserveSize);

    for (int d = 0; d < 3; ++d)
    {
        const int u = (d + 1) % 3;
        const int v = (d + 2) % 3;

        int x[3] = {};
        int q[3] = {};
        q[d]     = 1;

        Mask mask[dims[u] * dims[v]];

        for (x[d] = -1; x[d] < dims[d];)
        {
            // Determine mask
            int n = 0;
            for (x[v] = 0; x[v] < dims[v]; ++x[v])
                for (x[u] = 0; x[u] < dims[u]; ++x[u], ++n)
                {
                    const int c0[3] = {x[0],        x[1],        x[2]};
                    const int c1[3] = {x[0] + q[0], x[1] + q[1], x[2] + q[2]};
                    const int v0    = in(c0, dims) ? cells[c0[0]][c0[1]][c0[2]].v : 0;
                    const int v1    = in(c1, dims) ? cells[c1[0]][c1[1]][c1[2]].v : 0;
                    const int g0    = v0 ? cells[c0[0]][c0[1]][c0[2]].g : 0;
                    const int g1    = v1 ? cells[c1[0]][c1[1]][c1[2]].g : 0;
                    const int d     = v0 > 0 && v1 > 0 && g1 != g0 ?
                                      g1 - g0 : v0 - v1;
                    mask[n]         = {d, g0, g1};
                }

            ++x[d];
            n = 0;

            for (int j = 0; j < dims[v]; ++j)
                for (int i = 0; i < dims[u];)
                {
                    const Mask m = mask[n];
                    if (m.d)
                    {
                        // Dimensions
                        int w = 1, h = 1;
                        if (greedy)
                        {
                            // Compute width
                            for (w = 1; m == mask[n + w] && i + w < dims[u]; ++w)
                            {}
                            // Compute height
                            for (h = 1; j + h < dims[v]; ++h)
                                for (int k = 0; k < w; ++k)
                                    if (m != mask[n + k + h * dims[u]])
                                        goto dims_done;
                        }
                        dims_done:
                        x[u] = i;
                        x[v] = j;

                        // Query collapse constants for vertices
                        int cc[8][3];
                        collapseConstants(cc, m.d > 0 ? m.g0 : m.g1);

                        glm::ivec3 pos(x[0], x[1], x[2]);
                        if (m.d > 0) --pos[d];

                        glm::ivec3 size(1, 1, 1);
                        size[u] = w;
                        size[v] = h;

                        // Determine axis+direction [0, 5]
                        const int axis = d * 2 + (m.d > 0 ? 1 : 0);

                        // Emit quad
                        emitBoxFace(&mesh, scale,
                                     axis, cc, pos, size,
                                    {dims[0], dims[1], dims[2]},
                                     uvCube);

                        // Clear mask
                        for (int l = 0; l < h; ++l)
                            for (int k = 0; k < w; ++k)
                                mask[n + k + l * dims[u]] = {};

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
    return mesh;
}

} // namespace

Mesh_P_N_T_UV mesh(const Image& image, int smoothness, float scale)
{
    const Heightfield hfield(image, std::min(image.size().w, image.size().h));
    return meshGreedy(hfield, RectCube<float>(), smoothness, scale);
}

Mesh_P_N_T_UV mesh(const ImageCube& imageCube,
                   const RectCube<float>& uvCube,
                   int smoothness,
                   float scale)
{
    const Cubefield cfield(imageCube);
    auto mesh = meshGreedy(cfield, uvCube, scale, !smoothness);
    if (false/*smoothness > 0*/)
    {
        PTTIMEU("smooth", boost::milli);

        // Smoothing types
        using Indices       = std::vector<int>;
        using Neighbors     = std::vector<Indices>;
        using Displacements = std::vector<glm::vec3>;
        Neighbors neighbors(mesh.vertices.size());

        // Find neighbors
        const auto vc = int(mesh.vertices.size());
        const auto ic = int(mesh.indices.size());

        //#pragma omp parallel for
        for (int i = 0; i < ic; ++i)
        {
            const auto  i0 = mesh.indices[i];
            const auto& v0 = mesh.vertices[i0];
            for (int j = 0; j < ic; ++j)
            {
                const auto  i1 = mesh.indices[j];
                const auto& v1 = mesh.vertices[i1];
                if (v0.p == v1.p)
                {
                    // Triangle base-index
                    const auto ti = i1 % 3;
                    const auto ib = i1 - ti;
                    const auto n0 = ib + ((ti + 1) % 3);
                    const auto n1 = ib + ((ti + 2) % 3);
                    neighbors[i0].push_back(n0);
                    neighbors[i0].push_back(n1);
                }
            }
        }
        // Smooth iteratively
        const auto lambda = 0.5f;
        for (int c = 0; c < smoothness; ++c)
        {
            // Determine displacements
            Displacements displacements(vc, glm::vec3());

            //#pragma omp parallel for
            for (int i = 0; i < vc; ++i)
            {
                const auto& v0 = mesh.vertices[i];
                const auto& n  = neighbors[i];
                const auto nc  = int(n.size());
                if (nc)
                {
                    const auto w = 1.f / nc;
                    for (int j = 0; j < nc; ++j)
                    {
                        const auto& v1 = mesh.vertices[n[j]];
                        displacements[i] += w * (v0.p - v1.p);
                    }
                }
            }
            // Apply displacements
            const float s = !(c % 2) ? lambda : -lambda;

            #pragma omp parallel for
            for(int i = 0; i < vc; ++i)
                mesh.vertices[i].p += s * displacements[i];
        }
        // Recompute triangle normals and tangents
        //#pragma omp parallel for
        for (int i = 0; i < vc; i += 3)
        {
            auto& va = mesh.vertices[i + 0];
            auto& vb = mesh.vertices[i + 1];
            auto& vc = mesh.vertices[i + 2];
            auto n   = glm::normalize(glm::cross(vb.p - va.p, vc.p - va.p));
            auto t   = tangent({va.p,  vb.p,  vc.p},
                               {va.uv, vb.uv, vc.uv}, n);
            va.n = n;
            vb.n = n;
            vc.n = n;
            va.t = t;
            vb.t = t;
            vc.t = t;
        }
    }
    if (smoothness > 0)
        PTLOG(Info) << "v: "   << mesh.vertices.size()
                    << ", i: " << mesh.indices.size();

    return mesh;
}

} // namespace ImageMesher
} // namespace pt

