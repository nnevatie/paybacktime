#include "object.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "constants.h"

#include "geom/volume.h"
#include "img/color.h"

#include "common/json.h"
#include "common/log.h"

#include "common/metadata.h"

namespace bpt = boost::property_tree;

namespace pt
{
namespace
{

typedef std::function<glm::vec3(const glm::vec3&)> Projection;

void accumulateMaterial(mat::Emission& emission,
                        mat::Density& density,
                        const Projection& p,
                        const Image& depth,
                        const Image& albedo,
                        const Image& light,
                        const float objScale,
                        const float area)
{
    auto const cellArea     = c::cell::SIZE.x * c::cell::SIZE.z;
    constexpr auto exp      = c::object::EXPOSURE;
    constexpr auto rgbScale = 1.f / 255;
    auto const sizeDepth    = depth.size();
    auto const sizeLight    = light.size();
    auto const scaleLight   = (sizeLight.as<glm::vec2>() /
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
            auto xd     = int(x / float(sizeLight.w) * sizeDepth.w);
            auto d      = int(rowDepth[xd]);
            auto out    = (p({x / float(sizeLight.w - 1),
                              y / float(sizeLight.h - 1),
                              d / 255}) + 0.5f) * objScale;

            auto albedo = argbTuple(rowAlbedo[x]);
            auto rgb    = glm::vec3(albedo.rgb());
            auto rgbN   = glm::length(rgb) > 0.f ? glm::normalize(rgb) : rgb;
            auto emis   = exp * argbTuple(rowLight[x]).b * rgbScale / scaleLight;

            emission.at(out.x, out.y, out.z) += emis * rgbN;

            auto& d0  = density.at(out.x, out.y, out.z);
            auto  a0  = d0.a;
            auto  a1  = albedo.a / 255.f;
            auto  as  = std::min(a0, a1) < 1.f ? -1.f : 1.f;
            auto  a   = std::abs(a0) * glm::pow(a1, 0.75f * cellArea / area);
            auto rgb1 = a0 < 0.f ? 0.5f * (d0.rgb() + rgbN) : rgbN;
            d0 = glm::vec4(rgb1, as * a);
        }
    }
}

}

struct Meta
{
    Meta(const fs::path& path) :
        name(path.filename().string()),
        scale(c::object::SCALE)
    {
        const auto meta = readJson(path / "object.json");
        if (!meta.is_null())
        {
            scale  = meta.value("scale", c::object::SCALE);
            origin = glm::make_vec3(meta.value("origin",
                                    std::vector<float>(3)).data());
        }
        PTLOG(Info) << "meta: " << scale << "; " << origin.x << ", "
                                                 << origin.y << ", "
                                                 << origin.z;
    }

    std::string name;
    float       scale;
    glm::vec3   origin;
};

struct Object::Data
{
    Data(const fs::path& path, TextureStore* textureStore) :
        meta(path),
        model(path, textureStore, meta.scale),
        transparent(false),
        emissive(false)
    {}

    Meta            meta;
    Model           model;
    bool            transparent;
    bool            emissive;
    mat::Density    density;
    mat::Emission   emission;
};

Object::Object()
{}

Object::Object(const fs::path& path, TextureStore* textureStore) :
    d(std::make_shared<Data>(path, textureStore))
{
    updateTransparency();
    updateEmissivity();
    updateDensity();
    updateMaterial();
}

Object::operator bool() const
{
    return d.operator bool();
}

Object::operator==(const Object& other) const
{
    return d == other.d;
}

Object::operator!=(const Object& other) const
{
    return !operator==(other);
}

std::string Object::name() const
{
    return d->meta.name;
}

float Object::scale() const
{
    return d->meta.scale;
}

glm::vec3 Object::origin() const
{
    return d->meta.origin;
}

glm::mat4x4 Object::transform() const
{
    return glm::translate(d->meta.origin);
}

glm::vec3 Object::dimensions() const
{
    return d->model.dimensions() / scale();
}

bool Object::transparent() const
{
    return d->transparent;
}

Object& Object::updateTransparency()
{
    d->transparent = d->model.albedoCube()->transparent();
    return *this;
}

Model Object::model() const
{
    return d->model;
}

mat::Density Object::density() const
{
    return d->density;
}

Object& Object::updateDensity()
{
    const auto size = dimensions().xzy() / c::cell::SIZE.xzy();

    mat::Density map(glm::ceil(size));
    const Cubefield cfield(*d->model.depthCube());

    for (int z = 0; z < size.z; ++z)
        for (int y = 0; y < size.y; ++y)
            for (int x = 0; x < size.x; ++x)
            {
                int fx0   = x * cfield.width / size.x;
                int fy0   = y * cfield.depth / size.y;
                int fx1   = (x + 1) * cfield.width / size.x - 1;
                int fy1   = (y + 1) * cfield.depth / size.y - 1;
                int width = fx1 - fx0 + 1;
                int y0    = -d->meta.origin.y + z * c::cell::SIZE.y;
                int y1    = (z + 1) * c::cell::SIZE.y;

                int sum = 0;
                for (int fy = y0; fy < y1; ++fy)
                    for (int fx = fx0; fx <= fx1; ++fx)
                        for (int fz = fy0; fz <= fy1; ++fz)
                            if (cfield(fx, fy, fz) || cfield(fz, fy, fx))
                            {
                                ++sum;
                                break;
                            }

                float a = float(sum) / ((c::cell::SIZE.y + d->meta.origin.y) *
                                         width);

                map.at(x, y, z) = {1.f, 1.f, 1.f, a};
            }

    d->density = map;
    return *this;
}

bool Object::emissive() const
{
    return d->emissive;
}

mat::Emission Object::emission() const
{
    return d->emission;
}

Object& Object::updateEmissivity()
{
    d->emissive = d->model.lightCube()->emissive();
    return *this;
}

Object& Object::updateMaterial()
{
    const auto size     = dimensions().xzy();
    const auto cellSize = size / c::cell::SIZE.xzy();

    const auto pmax = glm::max(glm::vec3(0.f), cellSize - 1.f);
    auto cubeDepth  = d->model.depthCube();
    auto cubeAlbedo = d->model.albedoCube();
    auto cubeLight  = d->model.lightCube();

    mat::Emission emission(glm::ceil(cellSize));
    PTLOG(Info) << name() << " size: " << emission.size.x << "x"
                                       << emission.size.y << "x"
                                       << emission.size.z;
    const Projection projections[] =
    {
        // Front
        [&pmax](const glm::vec3& v)
        {return glm::vec3(pmax.x * v.x, pmax.y * v.z, pmax.z * v.y);},
        // Back
        [&pmax](const glm::vec3& v)
        {return glm::vec3(pmax.x * v.x, pmax.y - pmax.y * v.z, pmax.z * v.y);},
        // Left
        [&pmax](const glm::vec3& v)
        {return glm::vec3(pmax.x - pmax.x * v.z, pmax.y * v.x, pmax.z * v.y);},
        // Right
        [&pmax](const glm::vec3& v)
        {return glm::vec3(pmax.x * v.z, pmax.y * v.x, pmax.z * v.y);},
        // Top
        [&pmax](const glm::vec3& v)
        {return glm::vec3(pmax.x * v.x, pmax.y * v.y, pmax.z * v.z);},
        // Bottom
        [&pmax](const glm::vec3& v)
        {return glm::vec3(pmax.x * v.x, pmax.y * v.y, pmax.z * v.z);}
    };
    const float areas[] = {size.x * size.z,
                           size.x * size.z,
                           size.y * size.z,
                           size.y * size.z,
                           size.x * size.y,
                           size.x * size.y};

    for (int i = 0; i < 6; ++i)
    {
        PTLOG(Info) << i << " area: " << areas[i];
        const auto side = ImageCube::Side(i);
        accumulateMaterial(emission, d->density,
                           projections[i], cubeDepth->side(side),
                           cubeAlbedo->side(side),
                           cubeLight->side(side),
                           d->meta.scale, areas[i]);
    }
    d->emission = emission;
    return *this;
}

Object Object::flipped(TextureStore* textureStore) const
{
    if (d)
    {
        auto data   = std::make_shared<Data>(*d);
        data->model = data->model.flipped(textureStore, d->meta.scale);
        data->meta.origin.x += c::cell::SIZE.x; // TODO
        auto object = Object();
        object.d    = data;
        return object;
    }
    return Object();
}

} // namespace pt
