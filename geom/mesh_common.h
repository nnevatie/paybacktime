#pragma once

#include <array>

#include <glm/vec3.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/hash.hpp>

#include "mesh.h"
#include "rect.h"
#include "volume.h"

namespace std
{
// GLM vec3 pair hash
using glm_vec3_pair = std::pair<glm::vec3, glm::vec3>;
template<>
struct hash<glm_vec3_pair>
{
    size_t operator()(glm_vec3_pair const& v) const
    {
        return hash<glm::vec3>()(v.first);
    }
};
}

namespace pt
{

inline glm::vec3 tangent(const std::array<glm::vec3, 3>& pos,
                         const std::array<glm::vec2, 3>& uv,
                         const glm::vec3& n)
{
    auto e0   = pos[1] - pos[0];
    auto e1   = pos[2] - pos[0];
    auto duv0 = uv[1]  - uv[0];
    auto duv1 = uv[2]  - uv[0];
    auto f    = 1.0f / (duv0.x * duv1.y - duv0.y * duv1.x);
    glm::vec3 t(f * (e0 * duv1.y - e1 * duv0.y));
    return glm::normalize(-t - n * glm::dot(n, t));
}

inline int dominantAxis(const glm::vec3& v)
{
    static const glm::vec3 axes[] =
    {
        glm::vec3(-1,  0,  0),
        glm::vec3(+1,  0,  0),
        glm::vec3( 0, -1,  0),
        glm::vec3( 0, +1,  0),
        glm::vec3( 0,  0, -1),
        glm::vec3( 0,  0, +1)
    };

    int index   = -1;
    float score = 0.f;
    for (int i = 0; i < 6; ++i)
    {
        const auto d = glm::dot(axes[i], v);
        if (d >= score)
        {
            index = i;
            score = d;
        }
    }
    return index;
}

template <typename V>
inline bool visible(const V& vol, int x, int y, int z)
{
    return vol(x + 0, y + 0, z + 0) &&
           (
           !vol(x - 1, y + 0, z + 0) ||
           !vol(x + 1, y + 0, z + 0) ||
           !vol(x + 0, y - 1, z + 0) ||
           !vol(x + 0, y + 1, z + 0) ||
           !vol(x + 0, y + 0, z - 1) ||
           !vol(x + 0, y + 0, z + 1)
           );
}

inline void emitTri(Mesh_P_N_T_UV* g,
                    const std::array<glm::vec3, 3>& p,
                    const std::array<glm::vec2, 3>& uv)
{
    const auto p01 = p[1] - p[0];
    const auto p02 = p[2] - p[0];
    if (p01 != p02 && glm::length2(p01) > 0.f && glm::length2(p02) > 0.f)
    {
        const auto n  = glm::normalize(glm::cross(p01, p02));
        const auto t  = tangent(p, uv, n);
        const auto ib = Mesh_P_N_T_UV::Index(g->vertices.size());

        g->vertices.insert(g->vertices.end(), {{p[0], n, t, uv[0]},
                                               {p[1], n, t, uv[1]},
                                               {p[2], n, t, uv[2]}});

        g->indices.insert(g->indices.end(), {ib + 0, ib + 1, ib + 2});
    }
}

inline void emitQuad(Mesh_P_N_T_UV* g, int axis,
                     const glm::vec3& p, const glm::vec3& s, const glm::vec3& d,
                     const std::array<glm::vec3, 4>& v,
                     const RectCube<float>& uvCube)
{
    const int axisUV[6][2] =
    {
        {2, 1}, {2, 1},
        {0, 2}, {0, 2},
        {0, 1}, {0, 1},
    };
    const Rect<float> uvn(p[axisUV[axis][0]] / d[axisUV[axis][0]],
                          p[axisUV[axis][1]] / d[axisUV[axis][1]],
                          s[axisUV[axis][0]] / d[axisUV[axis][0]],
                          s[axisUV[axis][1]] / d[axisUV[axis][1]]);

    const int axisSide[6] = {2, 3, 5, 4, 1, 0};
    const Rect<float> uvr(uvCube[axisSide[axis]].rect(
                          uvn.x, uvn.y, uvn.size.w, uvn.size.h));

    const glm::vec2 uva(uvr.x,              uvr.y);
    const glm::vec2 uvb(uvr.x + uvr.size.w, uvr.y);
    const glm::vec2 uvc(uvr.x + uvr.size.w, uvr.y + uvr.size.h);
    const glm::vec2 uvd(uvr.x,              uvr.y + uvr.size.h);

    const glm::vec2 uv[6][4] =
    {
        {uvc, uvd, uva, uvb},
        {uvd, uvc, uvb, uva},
        {uva, uvb, uvc, uvd},
        {uva, uvd, uvc, uvb},
        {uvd, uvc, uvb, uva},
        {uva, uvb, uvc, uvd},
    };

    if (glm::length2(v[0] - v[2]) < glm::length2(v[1] - v[3]))
    {
        emitTri(g, {v[0], v[1], v[2]}, {uv[axis][0], uv[axis][1], uv[axis][2]});
        emitTri(g, {v[2], v[3], v[0]}, {uv[axis][2], uv[axis][3], uv[axis][0]});
    }
    else
    {
        emitTri(g, {v[1], v[2], v[3]}, {uv[axis][1], uv[axis][2], uv[axis][3]});
        emitTri(g, {v[3], v[0], v[1]}, {uv[axis][3], uv[axis][0], uv[axis][1]});
    }
}

} // namespace pt

