#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>

namespace pt
{

struct Transform
{
    using V = glm::vec3;

    V   pos;
    int rot;

    Transform() : rot(0)
    {}

    Transform(const V& pos, int rot = 0) : pos(pos), rot(rot)
    {}

    bool operator==(const Transform& other) const
    {
        return pos == other.pos && rot == other.rot;
    }

    bool operator!=(const Transform& other) const
    {
        return !operator==(other);
    }

    glm::mat4x4 translation() const
    {
        return glm::translate(pos);
    }

    glm::mat4x4 rotation() const
    {
        const auto ang = 0.5f * glm::half_pi<float>() * rot;
        return glm::rotate(ang, glm::vec3(0.f, 1.f, 0.f));
    }

    glm::mat4x4 matrix(const glm::vec3& size) const
    {
        auto hwh = glm::vec3(0.5f * size.x, 0.f, 0.5f * size.z);
        return glm::translate(translation() * rotation(), -hwh);
    }
};

struct Rotation
{
    glm::vec3 dim;
    int       rot;

    Rotation(const glm::ivec3& dim, int rot) : dim(dim), rot(rot & 0x03)
    {}

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
