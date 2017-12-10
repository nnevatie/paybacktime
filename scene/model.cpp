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

struct Cubes
{
    Cubes()
    {}

    Cubes(const fs::path& path, bool fallback = true) :
        depth((path / "*.png").generic_string(), 1, fallback),
        albedo((path / "albedo.*.png").generic_string(), 4, fallback),
        light(ImageCube((path / "light.*.png").generic_string(), 4, fallback).
              scaled(albedo)),
        normal(ImageCube((path / "normal.*.png").generic_string(), 1, fallback).
               normals().scaled(albedo))
    {}

    Cubes merged(const Cubes& cubes) const
    {
        Cubes merged(*this);
        merged.depth.merge(cubes.depth);
        merged.albedo.merge(cubes.albedo);
        merged.light.merge(cubes.light);
        merged.normal.merge(cubes.normal);
        return merged;
    }

    Cubes flipped() const
    {
        Cubes flipped;
        flipped.depth  = depth.flipped();
        flipped.albedo = albedo.flipped();
        flipped.light  = light.flipped();
        flipped.normal = normal.flipped();
        return flipped;
    }

    const Cubes& validate() const
    {
        for (auto cube : {&depth, &albedo, &light, &normal})
            cube->validate();

        const auto albedoSizes = albedo.sideSizes();
        for (auto cube : {&light, &normal})
            if (cube->sideSizes() != albedoSizes)
                throw std::runtime_error("Mismatching cube side sizes");

        return *this;
    }

    ImageCube depth, albedo, light, normal;
};

struct Model::Data
{
    std::time_t lastUpdated;
    fs::path    path;
    geom::Meta  geom;
    Cubes       cubes;

    gl::TextureAtlas::EntryCube atlasEntry;
    gl::Primitive               primitive;

    Data(const fs::path& path, const Model& base,
         TextureStore& textureStore, const geom::Meta& geom) :
         lastUpdated(0), path(path), geom(geom)
    {
        update(base, textureStore);
    }

    bool update(const Model& base, TextureStore& textureStore)
    {
        const auto modified = std::max(base ? lastModified(base.d->path) : 0,
                                       lastModified(path));
        if (modified > lastUpdated)
        {
            // Cube updates
            PTLOG(Info) << path << " base: " << (base ? base.d->path : "none");
            cubes = base ? base.d->cubes.merged(Cubes(path, false)) :
                           Cubes(path);

            // Cube validation
            cubes.validate();

            // Atlas removal
            if (gl::valid(atlasEntry))
            {
                textureStore.albedo.remove(atlasEntry);
                textureStore.light.remove(atlasEntry);
                textureStore.normal.remove(atlasEntry);
            }
            // Atlas insert
            atlasEntry  = textureStore.albedo.insert(cubes.albedo);
                          textureStore.light.insert(cubes.light);
                          textureStore.normal.insert(cubes.normal);
            // Update mesh
            auto mesh   = ImageMesher::mesh(cubes.depth, atlasEntry.second, geom);
            primitive   = gl::Primitive(mesh);
            lastUpdated = modified;

            PTLOG(Info) << path.string() << " tris: " << mesh.triangleCount();
            return true;
        }
        return false;
    }
};

Model::Model(const fs::path& path, const Model& base,
             TextureStore& textureStore, const geom::Meta& geom) :
    d(std::make_shared<Data>(path, base, textureStore, geom))
{
}

pt::Model::operator bool() const
{
    return d.operator bool();
}

glm::vec3 Model::dimensions() const
{
    const auto& depth = d->cubes.depth;
    return d->geom.scale * glm::vec3(depth.width(), depth.height(), depth.depth());
}

gl::Primitive Model::primitive() const
{
    return d->primitive;
}

const ImageCube& Model::depthCube() const
{
    return d->cubes.depth;
}

const ImageCube& Model::albedoCube() const
{
    return d->cubes.albedo;
}

const ImageCube& Model::lightCube() const
{
    return d->cubes.light;
}

bool Model::update(const Model& base, TextureStore& textureStore)
{
    return d->update(base, textureStore);
}

Model Model::flipped(TextureStore& textureStore) const
{
    if (d)
    {
        auto data        = std::make_shared<Data>(*d);
        auto model       = Model();
        model.d          = data;
        data->cubes      = d->cubes.flipped();
        data->atlasEntry = textureStore.albedo.insert(data->cubes.albedo);
                           textureStore.light.insert(data->cubes.light);
                           textureStore.normal.insert(data->cubes.normal);
        auto mesh        = ImageMesher::mesh(data->cubes.depth,
                                             data->atlasEntry.second,
                                             d->geom);
        data->primitive  = gl::Primitive(mesh);
        return model;
    }
    return Model();
}

} // namespace pt
