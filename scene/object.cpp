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

void accumulateEmission(Grid<glm::vec3>* map, const Projection& p,
                                              const Image& depth,
                                              const Image& albedo,
                                              const Image& light)
{
    auto const exp        = 0.025f;
    auto const rgbScale   = 1.f / 255;
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
            auto out   = p({x / float(sizeLight.w - 1) + 0.5f,
                            y / float(sizeLight.h - 1) + 0.5f,
                            d / 255.f + 0.5f});

            auto argb0 = map->at(out.x, out.y);
            auto argb1 = glm::vec3(argbTuple(rowAlbedo[x]).rgb()) * rgbScale;
            auto light = argbTuple(rowLight[x]).b * rgbScale / scaleLight;
            auto emis  = exp * light;
            auto argb2 = argb0 + emis * argb1;
            map->at(out.x, out.y) = argb2;
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

    Model           model;
    std::string     name;
    glm::vec3       origin;
    Grid<float>     visibility;
    Grid<glm::vec3> emission;
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

Grid<float> Object::visibility() const
{
    return d->visibility;
}

Object& Object::updateVisibility()
{
    const int height = 64;
    const auto size  = d->model.dimensions().xz() / 8.f;

    Grid<float> map(Size<int>(size.x, size.y));
    const Cubefield cfield(*d->model.depthCube());

    for (int y = 0; y < size.x; ++y)
        for (int x = 0; x < size.y; ++x)
        {
            int fx0   = x * cfield.width / size.x;
            int fy0   = y * cfield.depth / size.y;
            int fx1   = (x + 1) * cfield.width / size.x - 1;
            int fy1   = (y + 1) * cfield.depth / size.y - 1;
            int width = fx1 - fx0 + 1;

            int sum = 0;
            for (int fy = -d->origin.y; fy < height; ++fy)
                for (int fx = fx0; fx <= fx1; ++fx)
                    for (int fz = fy0; fz <= fy1; ++fz)
                        if (cfield(fx, fy, fz) || cfield(fz, fy, fx))
                        {
                            ++sum;
                            break;
                        }

            map.at(x, y) = float(sum) / ((height + d->origin.y) * width);
        }

    //image(map).write("c:/temp/vis_" + d->name + ".png");
    d->visibility = map;
    return *this;
}

Grid<glm::vec3> Object::emission() const
{
    return d->emission;
}

Object& Object::updateEmission()
{
    const auto size = d->model.dimensions().xz() / 8.f;
    const auto pmax = size - 1.f;
    auto cubeDepth  = d->model.depthCube();
    auto cubeAlbedo = d->model.albedoCube();
    auto cubeLight  = d->model.lightCube();

    Grid<glm::vec3> map(Size<int>(size.x, size.y));
    for (int i = 0; i < 6; ++i)
    {
        const auto side = ImageCube::Side(i);
        Projection p[] =
        {
            // Front
            [&pmax](const glm::vec3& v)
            {return glm::ivec2(pmax.x * v.x, pmax.y * v.z);},
            // Back
            [&pmax](const glm::vec3& v)
            {return glm::ivec2(pmax.x * v.x, pmax.y - pmax.y * v.z);},
            // Left
            [&pmax](const glm::vec3& v)
            {return glm::ivec2(pmax.x - pmax.x * v.z, pmax.y * v.x);},
            // Right
            [&pmax](const glm::vec3& v)
            {return glm::ivec2(pmax.x * v.z, pmax.y * v.x);},
            // Top
            [&pmax](const glm::vec3& v)
            {return glm::ivec2(pmax.x * v.x, pmax.y * v.y);},
            // Bottom
            [&pmax](const glm::vec3& v)
            {return glm::ivec2(pmax.x * v.x, pmax.y * v.y);}
        };
        accumulateEmission(&map, p[i], cubeDepth->side(side),
                                       cubeAlbedo->side(side),
                                       cubeLight->side(side));
    }
    d->emission = map;
    //image(map).write("c:/temp/emis_" + d->name + ".png");
    return *this;
}

} // namespace pt
