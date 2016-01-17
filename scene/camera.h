#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace pt
{

struct Camera
{
    Camera(const glm::vec3& target, float distance,
           float yaw, float pitch,
           float fov, float ar, float zNear, float zFar);

    glm::vec3 position() const;
    glm::vec3 forward() const;
    glm::vec3 right() const;
    glm::vec3 up() const;

    glm::mat4 matrix() const;
    glm::mat4 matrixProj() const;
    glm::mat4 matrixView() const;

    glm::vec4 rayEye(const glm::vec4& rayClip) const;
    glm::vec3 rayWorld(const glm::vec4& rayEye) const;

    glm::vec3 target;
    float     distance, yaw, pitch, fov, ar, zNear, zFar;
};

} // namespace pt
