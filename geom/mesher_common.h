#pragma once

#include <array>

#include <glm/vec3.hpp>

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
        glm::vec3( 0,  0, +1),
        glm::vec3( 0,  0, -1),
        glm::vec3(-1,  0,  0),
        glm::vec3(+1,  0,  0),
        glm::vec3( 0, +1,  0),
        glm::vec3( 0, -1,  0)
    };

    int axisIndex = -1;
    float score   = 0.f;
    for (int i = 0; i < 6; ++i)
    {
        const auto d = glm::dot(axes[i], v);
        if (d >= score)
        {
            axisIndex = i;
            score     = d;
        }
    }
    return axisIndex;
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

inline void emitQuad(Mesh_P_N_T_UV* g, int axis,
                     const glm::vec3& p, const glm::vec3& s, const glm::vec3& d,
                     const std::array<glm::vec3, 4>& vertices,
                     const RectCube<float>& uvCube)
{
    using V = glm::vec3;

    const auto& va = vertices[0];
    const auto& vb = vertices[1];
    const auto& vc = vertices[2];
    const auto& vd = vertices[3];

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
    const Rect<float> uv(uvCube[axisSide[axis]].rect(
                         uvn.x, uvn.y, uvn.size.w, uvn.size.h));

    const glm::vec2 uva(uv.x,             uv.y);
    const glm::vec2 uvb(uv.x + uv.size.w, uv.y);
    const glm::vec2 uvc(uv.x + uv.size.w, uv.y + uv.size.h);
    const glm::vec2 uvd(uv.x,             uv.y + uv.size.h);

    const glm::vec2 uvs[6][4] =
    {
        {uvc, uvd, uva, uvb},
        {uvd, uvc, uvb, uva},
        {uva, uvb, uvc, uvd},
        {uva, uvd, uvc, uvb},
        {uvd, uvc, uvb, uva},
        {uva, uvb, uvc, uvd},
    };

    const Mesh_P_N_T_UV::Index ib = g->vertices.size();

    const float l0 = glm::length2(va - vc);
    const float l1 = glm::length2(vb - vd);
    if (l0 < l1)
    {
        const V n0 = glm::normalize(glm::cross(vb - va, vc - va));
        const V n1 = glm::normalize(glm::cross(vd - vc, va - vc));
        const V t0 = tangent({va, vb, vc},
                             {uvs[axis][0], uvs[axis][1], uvs[axis][2]}, n0);
        const V t1 = tangent({vc, vd, va},
                             {uvs[axis][2], uvs[axis][3], uvs[axis][0]}, n1);

        g->vertices.insert(g->vertices.end(), {{va, n0, t0, uvs[axis][0]},
                                               {vb, n0, t0, uvs[axis][1]},
                                               {vc, n0, t0, uvs[axis][2]},
                                               {vc, n1, t1, uvs[axis][2]},
                                               {vd, n1, t1, uvs[axis][3]},
                                               {va, n1, t1, uvs[axis][0]}});
    }
    else
    {
        const V n0 = glm::normalize(glm::cross(vc - vb, vd - vb));
        const V n1 = glm::normalize(glm::cross(va - vd, vb - vd));
        const V t0 = tangent({vb, vc, vd},
                             {uvs[axis][1], uvs[axis][2], uvs[axis][3]}, n0);
        const V t1 = tangent({vd, va, vb},
                             {uvs[axis][3], uvs[axis][0], uvs[axis][1]}, n1);

        g->vertices.insert(g->vertices.end(), {{vb, n0, t0, uvs[axis][1]},
                                               {vc, n0, t0, uvs[axis][2]},
                                               {vd, n0, t0, uvs[axis][3]},
                                               {vd, n1, t1, uvs[axis][3]},
                                               {va, n1, t1, uvs[axis][0]},
                                               {vb, n1, t1, uvs[axis][1]}});
    }
    g->indices.insert(g->indices.end(), {ib + 0, ib + 1, ib + 2,
                                         ib + 3, ib + 4, ib + 5});
}

} // namespace pt

