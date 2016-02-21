#include "scene.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>

#include "common/log.h"

namespace pt
{

struct Scene::Data
{
    Data()
    {}

    std::vector<Item> items;
};

Scene::Scene() :
    d(std::make_shared<Data>())
{
}

Scene& Scene::add(const Item& item)
{
    d->items.emplace_back(item);
    return *this;
}

Scene::Item Scene::intersect(const glm::vec3& origin,
                             const glm::vec3& ray) const
{
    float di = 0;
    glm::intersectRayPlane(origin, ray, glm::vec3(), glm::vec3(0, 1, 0), di);
    const glm::vec3 pos = origin + di * ray;
    return {{}, pos};
}

gfx::Geometry::Instances Scene::geometryInstances() const
{
    gfx::Geometry::Instances instances;
    instances.reserve(d->items.size());
    for (const auto& item : d->items)
    {
        glm::mat4x4 m = glm::translate({}, item.pos);
        instances.push_back({item.obj.model().primitive(), m});
    }
    HCLOG(Info) << instances.size();
    return instances;
}

} // namespace pt
