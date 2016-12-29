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
        const glm::vec3 rotTr[] =
        {
            {0,  0,  0},
            {0,  0, 16},
            {16, 0, 16},
            {16, 0,  0}
        };
        auto ang = glm::half_pi<float>() * rot;
        auto mtr = glm::translate(tr + rotTr[rot % 4]);
        return glm::rotate(mtr, ang, glm::vec3(0, 1, 0));
    }
};

} // namespace
