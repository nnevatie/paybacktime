#include "scene.h"

#include <glm/gtx/intersect.hpp>

namespace pt
{

Scene::Scene()
{
}

Intersection Scene::intersect(const glm::vec3& origin,
                              const glm::vec3& ray) const
{
    float di = 0;
    glm::intersectRayPlane(origin, ray, glm::vec3(), glm::vec3(0, 1, 0), di);
    const glm::vec3 pos = origin + di * ray;
    return {pos, {}};
}

Scene& Scene::render()
{
    return *this;
}

} // namespace pt
