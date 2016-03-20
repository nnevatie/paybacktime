#include "model.h"

#include <functional>

#include <glm/glm.hpp>
#include <glm/vec2.hpp>

#include "common/log.h"
#include "geom/image_mesher.h"
#include "img/image_cube.h"
#include "gl/primitive.h"

namespace pt
{

struct Model::Data
{
    ImageCube cubeDepth,
              cubeAlbedo,
              cubeLight;

    gl::TextureAtlas::EntryCube entryAlbedo,
                                entryLight;
    gl::Primitive primitive;

    Data(const fs::path& path) :
        cubeDepth((path  / "*.png").string(), 1),
        cubeAlbedo((path / "albedo.*.png").string()),
        cubeLight((path  / "light.*.png").string())
    {
        HCLOG(Info) << path.string();
        cubeLight = cubeLight.scaled(cubeAlbedo);
    }
};

Model::Model(const fs::path& path, TextureStore* textureStore, float scale) :
    d(std::make_shared<Data>(path))
{
    d->entryAlbedo = textureStore->albedo.insert(d->cubeAlbedo);
    d->entryLight  = textureStore->light.insert(d->cubeLight);
    auto mesh      = ImageMesher::mesh(d->cubeDepth, d->entryAlbedo.second, scale);
    d->primitive   = gl::Primitive(mesh);
}

glm::vec3 Model::dimensions() const
{
    return {d->cubeDepth.width(), d->cubeDepth.height(), d->cubeDepth.depth()};
}

gl::Primitive Model::primitive() const
{
    return d->primitive;
}

const ImageCube* Model::depthCube() const
{
    return &d->cubeDepth;
}

const ImageCube* Model::albedoCube() const
{
    return &d->cubeAlbedo;
}

const ImageCube* Model::lightCube() const
{
    return &d->cubeLight;
}

} // namespace pt
