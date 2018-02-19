#pragma once

#include <memory>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "geom/transform.h"
#include "gl/texture.h"

#include "scene/object.h"
#include "scene/material_types.h"
#include "scene/horizon.h"

#include "lightmap.h"

namespace pt
{
namespace gfx
{

struct Lightmapper
{
    Lightmapper();

    gl::Texture* lightTexture() const;
    gl::Texture* incidenceTexture() const;

    Lightmap& map() const;

    Lightmapper& reset(const glm::ivec3& size = {});

    Lightmapper& add(const glm::vec3& pos,
                     const glm::mat4& rot,
                     const Object& obj);

    Lightmapper& add(const Transform& xform,
                     const Object& obj);

    Lightmapper& operator()(const Horizon& horizon);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gfx
} // namespace pt
