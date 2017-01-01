#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>

namespace pt
{

struct TransformTrRot
{
    typedef glm::vec3 V;

    V   tr;
    int rot;

    TransformTrRot() : rot(0)
    {}

    TransformTrRot(const V& tr, int rot = 0) : tr(tr), rot(rot)
    {}

    bool operator==(const TransformTrRot& other) const
    {
        return tr == other.tr && rot == other.rot;
    }

    bool operator!=(const TransformTrRot& other) const
    {
        return !operator==(other);
    }

    operator glm::mat4x4() const
    {
        auto ang = glm::half_pi<float>() * rot;
        auto mtr = glm::translate(tr);
        return glm::rotate(mtr, ang, glm::vec3(0, 1, 0));
    }
};

struct Rotation
{
    glm::vec3 dim;
    int       r;

    Rotation(const glm::ivec3& dim, int r) : dim(dim), r(r & 0x03)
    {}

    glm::ivec3 translation() const
    {
        switch (r)
        {
            case 1:  return {0,     0, dim.x};
            case 2:  return {dim.x, 0, dim.z};
            case 3:  return {dim.z, 0,     0};
            default: return {0,     0,     0};
        }
    }

    glm::ivec3 operator()(const glm::ivec3& v) const
    {
        switch (r)
        {
            case 0: return    v;
            case 1: return {  v.y,      -(v.x + 1), v.z};
            case 2: return {-(v.x + 1), -(v.y + 1), v.z};
            case 3: return {-(v.y + 1),   v.x, v.z};
        }
        return v;
    }
};

} // namespace
