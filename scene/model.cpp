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
    std::time_t lastUpdated;
    fs::path    path;
    float       scale;

    ImageCube   cubeDepth,
                cubeAlbedo,
                cubeLight,
                cubeNormal;

    gl::TextureAtlas::EntryCube atlasEntry;
    gl::Primitive               primitive;

    Data(const fs::path& path, TextureStore* textureStore, float scale) :
        lastUpdated(0), path(path), scale(scale)
    {
        PTLOG(Info) << path.string();
        update(textureStore);
    }

    Data& update(TextureStore* textureStore)
    {
        const auto modified = lastModified(path);
        if (modified > lastUpdated)
        {
            // Cube updates
            cubeDepth  = ImageCube((path / "*.png").string(),        1);
            cubeAlbedo = ImageCube((path / "albedo.*.png").string(), 4);
            cubeLight  = ImageCube((path / "light.*.png").string(),  4).
                         scaled(cubeAlbedo);
            cubeNormal = ImageCube((path / "normal.*.png").string(), 1).
                         normals().scaled(cubeAlbedo);

            // Atlas removal
            if (gl::valid(atlasEntry))
            {
                textureStore->albedo.remove(atlasEntry);
                textureStore->light.remove(atlasEntry);
                textureStore->normal.remove(atlasEntry);
            }
            // Atlas insert
            atlasEntry  = textureStore->albedo.insert(cubeAlbedo);
                          textureStore->light.insert(cubeLight);
                          textureStore->normal.insert(cubeNormal);
            // Update mesh
            auto mesh   = ImageMesher::mesh(cubeDepth, atlasEntry.second, scale);
            primitive   = gl::Primitive(mesh);
            lastUpdated = modified;
        }
        return *this;
    }
};

Model::Model()
{}

Model::Model(const fs::path& path, TextureStore* textureStore, float scale) :
    d(std::make_shared<Data>(path, textureStore, scale))
{
}

pt::Model::operator bool() const
{
    return d.operator bool();
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

Model& Model::update(TextureStore* textureStore)
{
    d->update(textureStore);
    return *this;
}

Model Model::flipped(TextureStore* textureStore, float scale) const
{
    if (d)
    {
        auto data        = std::make_shared<Data>(*d);
        auto model       = Model();
        model.d          = data;
        data->cubeDepth  = d->cubeDepth.flipped();
        data->cubeAlbedo = d->cubeAlbedo.flipped();
        data->cubeLight  = d->cubeLight.flipped();
        data->atlasEntry = textureStore->albedo.insert(data->cubeAlbedo);
                           textureStore->light.insert(data->cubeLight);
        auto mesh        = ImageMesher::mesh(data->cubeDepth,
                                             data->atlasEntry.second, scale);
        data->primitive  = gl::Primitive(mesh);
        return model;
    }
    return Model();
}

} // namespace pt
