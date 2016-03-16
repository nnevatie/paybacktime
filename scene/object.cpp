#include "object.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <glm/gtx/transform.hpp>

#include "geom/volume.h"
#include "img/color.h"

#include "common/log.h"

namespace bpt = boost::property_tree;

namespace pt
{

namespace
{

typedef std::function<glm::ivec2(const glm::vec3&)> Projection;

bpt::ptree readJson(const fs::path& path)
{
    try
    {
        bpt::ptree tree;
        bpt::read_json(path.string(), tree);
        return tree;
    }
    catch (const std::exception& e)
    {
        HCLOG(Warn) << e.what();
    }
    return bpt::ptree();
}

template <typename T>
T getVec(const bpt::ptree& tree, const std::string& key)
{
    try
    {
        T vec;
        auto children = tree.get_child(key);
        for (auto it = children.begin(); it != children.end(); ++it)
            vec[std::distance(children.begin(), it)] =
                it->second.get_value<float>();

        return vec;
    }
    catch (const std::exception& e)
    {}
    return {};
}

void accumulateEmission(Image* map, const Projection& p,
                                    const Image& depth,
                                    const Image& albedo,
                                    const Image& light)
{
    auto const exp        = 0.05f;
    auto const sizeDepth  = depth.size();
    auto const sizeLight  = light.size();
    auto const scaleLight = (sizeLight.as<glm::vec2>() /
                             sizeDepth.as<glm::vec2>()).x;

    for (int y = 0; y < sizeLight.h; ++y)
    {
        auto yd = int(y / float(sizeLight.h) * sizeDepth.h);

        const uint8_t* __restrict__ rowDepth =
            depth.bits(0, yd);

        const uint32_t* __restrict__ rowAlbedo =
            reinterpret_cast<const uint32_t*>(albedo.bits(0, y));

        const uint32_t* __restrict__ rowLight =
            reinterpret_cast<const uint32_t*>(light.bits(0, y));

        for (int x = 0; x < sizeLight.w; ++x)
        {
            auto xd    = int(x / float(sizeLight.w) * sizeDepth.w);
            auto d     = int(rowDepth[xd]);
            auto out   = p({x / float(sizeLight.w),
                            y / float(sizeLight.h),
                            d / 256.f});

            auto argb0 = glm::vec4(argbTuple(*map->bits<int32_t>(out.x, out.y)));
            auto argb1 = glm::vec4(argbTuple(rowAlbedo[x]));
            auto emis  = (exp / 255) * argbTuple(rowLight[x]).b / scaleLight;
            auto argb2 = glm::uvec4(argb0 + emis * argb1);

            *map->bits<int32_t>(out.x, out.y) =
                argb(glm::min(glm::uvec4(255), argb2));
        }
    }
}

}

struct Object::Data
{
    Data(const fs::path& path, TextureStore* textureStore) :
        model(path, textureStore)
    {
        bpt::ptree tree(readJson(path / "object.json"));
        // TODO: Parse json properties
        name   = path.filename().string();
        origin = getVec<glm::vec3>(tree, "origin");
    }

    Model       model;
    std::string name;
    glm::vec3   origin;
    Image       visibility;
    Image       emission;
};

Object::Object()
{}

Object::Object(const fs::path& path, TextureStore* textureStore) :
    d(std::make_shared<Data>(path, textureStore))
{
    updateVisibility();
    updateEmission();
}

Object::operator==(const Object& other) const
{
    return d == other.d;
}

Object::operator!=(const Object& other) const
{
    return !operator==(other);
}

Object::operator bool() const
{
    return d.operator bool();
}

std::string Object::name() const
{
    return d->name;
}

glm::vec3 Object::origin() const
{
    return d->origin;
}

glm::mat4x4 Object::transform() const
{
    return glm::translate(d->origin);
}

Model Object::model() const
{
    return d->model;
}

Image Object::visibility() const
{
    return d->visibility;
}

Object& Object::updateVisibility()
{
    const int height = 64;
    const int step   = 4;
    const auto size  = d->model.dimensions().xz() / 8.f;

    Image map(Size<int>(size.x, size.y), 1);
    map.fill(0x00000000);

    const Cubefield cfield(*d->model.depthCube());
    for (int z = 0; z < cfield.depth; ++z)
        for (int x = 0; x < cfield.width; ++x)
        {
            int sum = 0;
            for (int y = -d->origin.y; y < height; y += step)
                sum += cfield(x, y, z);

            auto xo = int(x / float(cfield.width) * size.x);
            auto yo = int(z / float(cfield.depth) * size.y);
            auto d0 = float(*map.bits<uint8_t>(xo, yo));
            auto d1 = 255 * step * float(sum) / (height + d->origin.y);
            *map.bits<uint8_t>(xo, yo) = glm::max(d0, d1);
        }

    d->visibility = map;
    return *this;
}

Image Object::emission() const
{
    return d->emission;
}

Object& Object::updateEmission()
{
    const auto size = d->model.dimensions().xz() / 8.f;
    Image map(Size<int>(size.x, size.y), 4);
    map.fill(0x00000000);

    auto cubeDepth  = d->model.depthCube();
    auto cubeAlbedo = d->model.albedoCube();
    auto cubeLight  = d->model.lightCube();

    for (int i = 0; i < 6; ++i)
    {
        const auto side = ImageCube::Side(i);
        Projection p[] =
        {
            // Front
            [&size](const glm::vec3& v)
            {return glm::ivec2(size.x * v.x, size.y * v.z);},
            // Back
            [&size](const glm::vec3& v)
            {return glm::ivec2(size.x * v.x, size.y - size.y * v.z);},
            // Left
            [&size](const glm::vec3& v)
            {return glm::ivec2(size.x - size.x * v.z, size.y * v.x);},
            // Right
            [&size](const glm::vec3& v)
            {return glm::ivec2(size.x * v.z, size.y * v.x);},
            // Top
            [&size](const glm::vec3& v)
            {return glm::ivec2(size.x * v.x, size.y * v.y);},
            // Bottom
            [&size](const glm::vec3& v)
            {return glm::ivec2(size.x * v.x, size.y * v.y);}
        };
        accumulateEmission(&map, p[i], cubeDepth->side(side),
                                       cubeAlbedo->side(side),
                                       cubeLight->side(side));
    }
    d->emission = map;
    return *this;
}

} // namespace pt
