#pragma once

#include <array>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

namespace pt
{
namespace MeshSimplifier
{
using Scalar = float;

struct SymMat
{
    SymMat(Scalar c = 0)
    {
        m.fill(c);
    }

    SymMat(Scalar m11, Scalar m12, Scalar m13, Scalar m14,
                       Scalar m22, Scalar m23, Scalar m24,
                                   Scalar m33, Scalar m34,
                                               Scalar m44)
    {
        m[0] = m11; m[1] = m12; m[2] = m13; m[3] = m14;
                    m[4] = m22; m[5] = m23; m[6] = m24;
                                m[7] = m33; m[8] = m34;
                                            m[9] = m44;
    }

    SymMat(Scalar a, Scalar b, Scalar c, Scalar d)
    {
        m[0] = a * a; m[1] = a * b; m[2] = a * c; m[3] = a * d;
                      m[4] = b * b; m[5] = b * c; m[6] = b * d;
                                    m[7] = c * c; m[8] = c * d;
                                                  m[9] = d * d;
    }

    Scalar operator[](int c) const
    {
        return m[c];
    }

    Scalar det(int a11, int a12, int a13,
               int a21, int a22, int a23,
               int a31, int a32, int a33) const
    {
        return m[a11] * m[a22] * m[a33] +
               m[a13] * m[a21] * m[a32] +
               m[a12] * m[a23] * m[a31] -
               m[a13] * m[a22] * m[a31] -
               m[a11] * m[a23] * m[a32] -
               m[a12] * m[a21] * m[a33];
    }

    const SymMat operator+(const SymMat& n) const
    {
        return {m[0] + n[0], m[1] + n[1], m[2] + n[2], m[3] + n[3],
                             m[4] + n[4], m[5] + n[5], m[6] + n[6],
                                          m[7] + n[7], m[8] + n[8],
                                                       m[9] + n[9]};
    }

    SymMat& operator+=(const SymMat& n)
    {
        m[0] += n[0]; m[1] += n[1]; m[2] += n[2]; m[3] += n[3];
        m[4] += n[4]; m[5] += n[5]; m[6] += n[6]; m[7] += n[7];
        m[8] += n[8]; m[9] += n[9];
        return *this;
    }

    std::array<Scalar, 10> m;
};

struct Triangle
{
    Triangle() :
        deleted(0), dirty(0), err{0, 0, 0, 0}
    {}

    Triangle(int i0, int i1, int i2, const glm::vec3& n) :
        v{i0, i1, i2},
        deleted(0), dirty(0), err{0, 0, 0, 0},
        n(n)
    {}

    int       v[3], deleted, dirty;
    Scalar    err[4];
    glm::vec3 n;
};

struct Vertex
{
    Vertex()
    {}

    Vertex(const glm::vec3& p) : p(p)
    {}

    glm::vec3 p;
    SymMat    q;
    int       tstart, tcount, border;
};

struct Ref
{
    int tid, tvertex;
};

namespace
{

Scalar vertexError(const SymMat& q, Scalar x, Scalar y, Scalar z)
{
    return q[0] * x * x + 2 * q[1] * x * y + 2 * q[2] * x * z +
           2 * q[3] * x + q[4] * y * y + 2 * q[5] * y * z +
           2 * q[6] * y + q[7] * z * z + 2 * q[8] * z + q[9];
}

} // namespace

struct Simplifier
{
    using Tris  = std::vector<Triangle>;
    using Verts = std::vector<Vertex>;
    using Refs  = std::vector<Ref>;

    Tris&  triangles;
    Verts& vertices;
    Refs   refs;

    Simplifier(Tris& triangles, Verts& vertices) :
        triangles(triangles), vertices(vertices)
    {}

    void simplify(int iterCount, float threshold, float scaler)
    {
        int deletedTriangles = 0;
        std::vector<int> deleted0, deleted1;

        for (auto& t : triangles)
            t.deleted = 0;

        for (int iter = 0; iter < iterCount; ++iter)
        {
            // Update mesh
            updateMesh(iter);

            // Clear dirty flags
            for (auto& t : triangles)
                t.dirty = 0;

            // Iterate over triangles
            for (const auto& t : triangles)
            {
                if(t.err[3] > threshold || t.deleted || t.dirty)
                    continue;

                for (int i = 0; i < 3; ++i)
                    if(t.err[i] < threshold)
                    {
                        int i0   = t.v[i];
                        int i1   = t.v[(i + 1) % 3];
                        auto& v0 = vertices[i0];
                        auto& v1 = vertices[i1];

                        // Border check
                        if(v0.border != v1.border)
                            continue;

                        // Compute vertex to collapse to
                        glm::vec3 p;
                        calcError(i0, i1, p);

                        deleted0.resize(v0.tcount);
                        deleted1.resize(v1.tcount);

                        // Do not remove if flipped
                        if (flipped(p, i0, i1, v0, v1, deleted0)) continue;
                        if (flipped(p, i1, i0, v1, v0, deleted1)) continue;

                        // Not flipped, remove edge
                        v0.p = p;
                        v0.q = v1.q + v0.q;
                        int tstart = int(refs.size());

                        updateTriangles(i0, v0, deleted0, deletedTriangles);
                        updateTriangles(i0, v1, deleted1, deletedTriangles);

                        int tcount = int(refs.size()) - tstart;
                        if(tcount <= v0.tcount)
                        {
                            if(tcount)
                                std::copy(&refs[tstart], &refs[tstart] + tcount,
                                          &refs[v0.tstart]);
                        }
                        else
                            v0.tstart = tstart;

                        v0.tcount = tcount;
                        break;
                    }
            }
            threshold *= scaler;
        }
        compactMesh();
    }

    bool flipped(const glm::vec3& p, int /*i0*/, int i1,
                 const Vertex& v0, const Vertex& /*v1*/,
                 std::vector<int>& deleted) const
    {
        int bordercount=0;
        for (int k = 0; k < v0.tcount; ++k)
        {
            const Triangle& t = triangles[refs[v0.tstart + k].tid];
            if (t.deleted) continue;

            int s   = refs[v0.tstart + k].tvertex;
            int id1 = t.v[(s + 1) % 3];
            int id2 = t.v[(s + 2) % 3];

            if(id1 ==i1 || id2 == i1)
            {
                bordercount++;
                deleted[k] = 1;
                continue;
            }

            auto d1 = glm::normalize(vertices[id1].p - p);
            auto d2 = glm::normalize(vertices[id2].p - p);
            if(std::abs(glm::dot(d1, d2)) > 0.999f)
                return true;

            auto n = glm::normalize(glm::cross(d1, d2));
            deleted[k] = 0;

            if(glm::dot(n, t.n) < 0.2f)
                return true;
        }
        return false;
    }

    Scalar calcError(int id_v1, int id_v2, glm::vec3& result)
    {
        SymMat q      = vertices[id_v1].q + vertices[id_v2].q;
        bool   border = vertices[id_v1].border & vertices[id_v2].border;
        Scalar error  = 0.f;
        Scalar det    = q.det(0, 1, 2, 1, 4, 5, 2, 5, 7);

        if (det != 0.f && !border)
        {
            result.x = -1 / det * q.det(1, 2, 3, 4, 5, 6, 5, 7, 8);
            result.y =  1 / det * q.det(0, 2, 3, 1, 5, 6, 2, 7, 8);
            result.z = -1 / det * q.det(0, 1, 3, 1, 4, 6, 2, 5, 8);
            error    = vertexError(q, result.x, result.y, result.z);
        }
        else
        {
            glm::vec3 p1 = vertices[id_v1].p;
            glm::vec3 p2 = vertices[id_v2].p;
            glm::vec3 p3 = 0.5f * (p1 + p2);
            Scalar error1 = vertexError(q, p1.x, p1.y, p1.z);
            Scalar error2 = vertexError(q, p2.x, p2.y, p2.z);
            Scalar error3 = vertexError(q, p3.x, p3.y, p3.z);
            error  = std::min(error1, std::min(error2, error3));
            result = error == error1 ? p1 :
                     error == error2 ? p2 : p3;
        }
        return error;
    }

    void updateTriangles(int i0, const Vertex &v,
                         std::vector<int>& deleted,
                         int& deletedTriangles)
    {
        glm::vec3 p;
        for (int i = 0; i < v.tcount; ++i)
        {
            auto& r = refs[v.tstart + i];
            auto& t = triangles[r.tid];
            if(t.deleted)
                continue;

            if(deleted[i])
            {
                t.deleted = 1;
                ++deletedTriangles;
                continue;
            }

            t.v[r.tvertex] = i0;
            t.dirty  = 1;
            t.err[0] = calcError(t.v[0], t.v[1], p);
            t.err[1] = calcError(t.v[1], t.v[2], p);
            t.err[2] = calcError(t.v[2], t.v[0], p);
            t.err[3] = std::min(t.err[0], std::min(t.err[1], t.err[2]));
            refs.push_back(r);
        }
    }

    void updateMesh(int iter)
    {
        if(iter > 0)
        {
            int dst = 0;
            for (auto& t : triangles)
                if(!t.deleted)
                    triangles[dst++] = t;

            triangles.resize(dst);
        }
        //if (iter == 0)
        {
            for (auto& v : vertices)
                v.q = SymMat(0.f);

            for (auto& t : triangles)
            {
                const auto& p0 = vertices[t.v[0]].p;
                const auto& p1 = vertices[t.v[1]].p;
                const auto& p2 = vertices[t.v[2]].p;
                t.n = glm::normalize(glm::cross(p1 - p0, p2 - p0));
                for (int j = 0; j < 3; ++j)
                    vertices[t.v[j]].q += SymMat(t.n.x, t.n.y, t.n.z,
                                                 glm::dot(-t.n, p0));
            }
            for (auto& t : triangles)
            {
                glm::vec3 p;
                for (int j = 0; j < 3; ++j)
                    t.err[j] = calcError(t.v[j], t.v[(j + 1) % 3], p);

                t.err[3] = std::min(t.err[0], std::min(t.err[1], t.err[2]));
            }
        }

        for (auto& v : vertices)
        {
            v.tstart = 0;
            v.tcount = 0;
        }
        for (auto& t : triangles)
        {
            for (int j = 0; j < 3; ++j)
                vertices[t.v[j]].tcount++;
        }
        int tstart = 0;
        for (auto& v : vertices)
        {
            v.tstart = tstart;
            tstart  += v.tcount;
            v.tcount = 0;
        }

        refs.resize(triangles.size() * 3);
        for (int i = 0, c = int(triangles.size()); i < c; ++i)
        {
            auto& t = triangles[i];
            for (int j = 0; j < 3; ++j)
            {
                auto& v = vertices[t.v[j]];
                refs[v.tstart + v.tcount].tid    = i;
                refs[v.tstart + v.tcount].tvertex= j;
                ++v.tcount;
            }
        }

        //if (iter == 0)
        {
            std::vector<int> vcount, vids;

            for (auto& v : vertices)
                v.border = 0;

            for (auto& v : vertices)
            {
                vcount.clear();
                vids.clear();
                for (int j = 0; j < v.tcount; ++j)
                {
                    int k   = refs[v.tstart + j].tid;
                    auto& t = triangles[k];
                    for (int k = 0; k < 3; ++k)
                    {
                        int ofs = 0, id = t.v[k];
                        while(ofs < int(vcount.size()))
                        {
                            if(vids[ofs] == id) break;
                            ++ofs;
                        }
                        if(ofs == int(vcount.size()))
                        {
                            vcount.push_back(1);
                            vids.push_back(id);
                        }
                        else
                            ++vcount[ofs];
                    }
                }
                for (int j = 0, c = int(vcount.size()); j < c; ++j)
                    if(vcount[j] == 1)
                        vertices[vids[j]].border = 1;
            }
        }
    }

    void compactMesh()
    {
        int dst = 0;
        for (auto& v : vertices)
            v.tcount = 0;

        for (auto& t : triangles)
            if(!t.deleted)
            {
                triangles[dst++] = t;
                for (int i = 0; i < 3; ++i)
                    vertices[t.v[i]].tcount = 1;
            }

        triangles.resize(dst);

        dst = 0;
        for (auto& v : vertices)
            if(v.tcount)
            {
                v.tstart = dst;
                vertices[dst++].p = v.p;
            }

        for (auto& t : triangles)
            for (int i = 0; i < 3; ++i)
                t.v[i] = vertices[t.v[i]].tstart;

        vertices.resize(dst);
    }
};

} // namespace MeshSimplifier
} // namespace pt
