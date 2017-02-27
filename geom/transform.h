#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>

namespace pt
{

struct PosRotation
{
    typedef glm::vec3 V;

    V   pos;
    int rot;

    PosRotation() : rot(0)
    {}

    PosRotation(const V& pos, int rot = 0) : pos(pos), rot(rot)
    {}

    bool operator==(const PosRotation& other) const
    {
        return pos == other.pos && rot == other.rot;
    }

    bool operator!=(const PosRotation& other) const
    {
        return !operator==(other);
    }

    operator glm::mat4x4() const
    {
        auto ang = glm::half_pi<float>() * rot;
        auto mtr = glm::translate(pos);
        return glm::rotate(mtr, ang, glm::vec3(0, 1, 0));
    }
};

struct Rotation
{
    glm::vec3 dim;
    int       rot;

    Rotation(const glm::ivec3& dim, int rot) : dim(dim), rot(rot & 0x03)
    {}

    glm::ivec3 translation() const
    {
        switch (rot)
        {
            case 1:  return {0,     0, dim.x};
            case 2:  return {dim.x, 0, dim.z};
            case 3:  return {dim.z, 0,     0};
            default: return {0,     0,     0};
        }
    }

    glm::ivec3 operator()(const glm::ivec3& v) const
    {
        switch (rot)
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
