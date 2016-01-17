#include "camera.h"

#include <glm/ext.hpp>

namespace pt
{

Camera::Camera(const glm::vec3& target, float distance,
               float yaw, float pitch,
               float fov, float ar, float zNear, float zFar) :
    target(target), distance(distance),
    yaw(yaw), pitch(pitch),
    fov(fov), ar(ar), zNear(zNear), zFar(zFar)
{
}

glm::vec3 Camera::position() const
{
    return target - distance * forward();
}

glm::vec3 Camera::forward() const
{
    return glm::normalize(glm::vec3(std::cos(yaw) * std::cos(pitch),
                                    std::sin(pitch),
                                   -std::sin(yaw) * std::cos(pitch)));
}

glm::vec3 Camera::right() const
{
    return glm::normalize(glm::cross(forward(), up()));
}

glm::vec3 Camera::up() const
{
    return glm::vec3(0, 1, 0);
}

glm::mat4 Camera::matrix() const
{
    return matrixProj() * matrixView();
}

glm::mat4 Camera::matrixProj() const
{
    return glm::perspective(fov, ar, zNear, zFar);
}

glm::mat4 Camera::matrixView() const
{
    return glm::lookAt(position(), target, up());
}

glm::vec4 Camera::rayEye(const glm::vec4& rayClip) const
{
    const glm::vec4 eye = glm::inverse(matrixProj()) * rayClip;
    return glm::vec4(eye.x, eye.y, -1.f, 0.f);
}

glm::vec3 Camera::rayWorld(const glm::vec4& rayEye) const
{
    glm::vec3 world = (glm::inverse(matrixView()) * rayEye).xyz();
    return glm::normalize(world);
}

} // namespace pt
