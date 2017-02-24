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

glm::vec4 Camera::infoClip() const
{
    return {zNear * zFar, zNear - zFar, zFar, 1.f};
}

glm::vec4 Camera::infoProj() const
{
    const auto proj = matrixProj();
    const auto p    = glm::value_ptr(proj);
    return {
        2.0f / (p[4 * 0 + 0]),
        2.0f / (p[4 * 1 + 1]),
      -(1.0f -  p[4 * 2 + 0]) / p[4 * 0 + 0],
      -(1.0f +  p[4 * 2 + 1]) / p[4 * 1 + 1]
    };
}

glm::vec4 Camera::eye(const glm::vec4& clip) const
{
    const glm::vec4 eye = glm::inverse(matrixProj()) * clip;
    return glm::vec4(eye.x, eye.y, -1.f, 0.f);
}

Ray Camera::world(const glm::vec4& rayEye) const
{
    glm::vec3 world = (glm::inverse(matrixView()) * rayEye).xyz();
    return {position(), glm::normalize(world)};
}

} // namespace pt
