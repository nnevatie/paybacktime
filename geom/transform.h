#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>

#include "constants.h"

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
        return translation(pos);
    }

    glm::mat4x4 rotation() const
    {
        return rotation(c::scene::UP, rot);
    }

    glm::mat4x4 matrix(const glm::vec3& size) const
    {
        auto hwh = glm::vec3(0.5f * size.x, 0.f, 0.5f * size.z);
        return glm::translate(translation() * rotation(), -hwh);
    }

    static glm::mat4x4 translation(const glm::vec3& pos)
    {
        return glm::translate(pos);
    }

    static glm::mat4x4 rotation(const glm::vec3& axis, int rot)
    {
        const auto a = glm::two_pi<float>() * rot / c::scene::ROT_TICKS;
        return glm::rotate(a, axis);
    }
};

} // namespace
