#include "object.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "geom/meta.h"
#include "geom/volume.h"
#include "img/color.h"

#include "common/metadata.h"
#include "common/json.h"
#include "common/log.h"

#include "constants.h"

namespace bpt = boost::property_tree;

namespace pt
{
namespace
{
using Projection = std::function<glm::vec3(const glm::vec3&)>;

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

struct Meta
{
    Meta(const Object::Path& path) :
        id(Object::pathId(path)),
        name(path.first.filename().string())
    {
        const auto meta = readJson(path.first / c::object::METAFILE);
        if (!meta.is_null())
        {
            base       = meta.value(c::object::meta::BASE, Object::Id());
            geom.scale = meta.value(c::object::meta::SCALE, 1.f);
            {
                // Smooth deformer
                const auto it = meta.find(c::object::meta::SMOOTH);
                if (it != meta.end())
                {
                    const auto smooth = *it;
                    if (smooth.is_number_integer())
                        geom.smooth.iterations = smooth;
                    else
                    if (smooth.is_array())
                    {
                        geom.smooth.iterations = smooth[0];
                        geom.smooth.strength   = smooth[1];
                    }
                }
            }
            {
                // Simplify deformer
                const auto it = meta.find(c::object::meta::SIMPLIFY);
                if (it != meta.end())
                {
                    const auto simplify = *it;
                    if (simplify.is_number_integer())
                        geom.simplify.iterations = simplify;
                    else
                    if (simplify.is_array())
                    {
                        geom.simplify.iterations = simplify[0];
                        geom.simplify.strength   = simplify[1];
                        geom.simplify.scale      = simplify[2];
                    }
                }
            }
            // Origin
            origin = geom.scale *
                     glm::make_vec3(meta.value(c::object::meta::ORIGIN,
                                               std::vector<float>(3)).data());
            // Children
            childIds = meta.value(c::object::meta::CHILDREN, Object::Ids());
            for (auto& childId : childIds)
                childId = id + "/" + childId;
        }
    }

    Object::Id  id;
    Object::Id  base;
    std::string name;
    glm::vec3   origin;
    geom::Meta  geom;
    Object::Ids childIds;
};

} // namespace

struct Object::Data
{
    Data(const Path& path, const Resolver& resolver,
         TextureStore& textureStore) :
        meta(path),
        transparent(false),
        emissive(false)
    {
        if (meta.childIds.empty())
        {
            const auto base = baseObject(resolver, textureStore);
            model = Model(path.first, base ? base.model() : Model(),
                          textureStore, meta.geom);
        }
    }

    Object baseObject(const Resolver& resolver,
                      TextureStore& textureStore) const
    {
        return !meta.base.empty() && resolver ?
                resolver(meta.base, textureStore) : Object();
    }

    Meta          meta;
    Model         model;
    bool          transparent;
    bool          emissive;
    mat::Density  density;
    mat::Emission emission;
    Object        parent;
    Objects       children;
};

Object::Object(const Path& path, const Resolver& resolver,
               TextureStore& textureStore) :
    d(std::make_shared<Data>(path, resolver, textureStore))
{
    // Resolve children
    for (const auto& childId : d->meta.childIds)
    {
        auto child = resolver(childId, textureStore);
        child.setParent(*this);
        d->children.emplace_back(child);
    }
    updateApproximation();
}

Object::operator bool() const
{
    return d.operator bool();
}

bool Object::operator==(const Object& other) const
{
    return d == other.d;
}

bool Object::operator!=(const Object& other) const
{
    return !operator==(other);
}

std::string Object::id() const
{
    return d->meta.id;
}

std::string Object::name() const
{
    return d->meta.name;
}

glm::vec3 Object::origin() const
{
    return d->meta.origin;
}

Transform Object::parentTransform(const Transform& xform) const
{
    return parent() ? (xform - origin()) : xform;
}

glm::mat4x4 Object::childMatrix(const Transform& xform) const
{
    return (parent() ? (xform + origin()) : xform).matrix(dimensions());
}

glm::vec3 Object::dimensions() const
{
    Aabb aabb(d->model ? d->model.dimensions() : glm::zero<glm::vec3>());
    for (const auto& child : d->children)
        aabb |= Aabb(child.dimensions());

    return aabb.size();
}

bool Object::transparent() const
{
    return d->transparent;
}

Object& Object::updateTransparency()
{
    d->transparent = d->model ? d->model.albedoCube().transparent() : false;
    return *this;
}

Object Object::parent() const
{
    return d->parent;
}

Object& Object::setParent(const Object& object)
{
    d->parent = object;
    return *this;
}

Objects Object::hierarchy() const
{
    Objects objects;
    objects.reserve(1 + d->children.size());
    objects.emplace_back(*this);
    objects.insert(objects.end(), d->children.begin(), d->children.end());
    return objects;
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
    if (!d->model)
        return *this;

    const auto size = dimensions().xzy() / c::cell::SIZE.xzy();

    mat::Density map(glm::ceil(size));
    const Cubefield cfield(d->model.depthCube());

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
    if (d->model)
        d->emissive = d->model.lightCube().emissive();
    return *this;
}

mat::Pulse Object::pulse() const
{
    return d->meta.id == "screen2" ?
           mat::Pulse(1.f, 0.5f) :
           d->meta.id == "screen" ?
           mat::Pulse(1.f, 1.f) :
           mat::Pulse(0.f, 0.f);
}

Object& Object::updateMaterial()
{
    if (!d->model)
        return *this;

    const auto size     = dimensions().xzy();
    const auto cellSize = size / c::cell::SIZE.xzy();

    const auto pmax = glm::max(glm::vec3(0.f), cellSize - 1.f);
    auto cubeDepth  = d->model.depthCube();
    auto cubeAlbedo = d->model.albedoCube();
    auto cubeLight  = d->model.lightCube();

    mat::Emission emission(glm::ceil(cellSize));
    //PTLOG(Info) << name() << " size: " << emission.size.x << "x"
    //                                   << emission.size.y << "x"
    //                                   << emission.size.z;
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
        const auto side = ImageCube::Side(i);
        accumulateMaterial(emission, d->density,
                           projections[i], cubeDepth.side(side),
                           cubeAlbedo.side(side),
                           cubeLight.side(side),
                           d->meta.geom.scale, areas[i]);
    }
    d->emission = emission;
    return *this;
}

Object& Object::updateApproximation()
{
    updateTransparency();
    updateEmissivity();
    updateDensity();
    updateMaterial();
    return *this;
}

bool Object::update(const Resolver& resolver, TextureStore& textureStore)
{
    auto base = d->baseObject(resolver, textureStore);
    if (base) base.update(resolver, textureStore);

    if (d->model && d->model.update(base ? base.d->model : Model(), textureStore))
    {
        updateApproximation();
        return true;
    }
    return false;
}

Object Object::flipped(TextureStore& textureStore) const
{
    if (d)
    {
        // Clone object
        auto object = Object();
        object.d    = std::make_shared<Data>(*d);

        // Flip model
        if (d->model)
            object.d->model = d->model.flipped(textureStore);

        // Flip origin
        object.d->meta.origin.x = -d->model.dimensions().x - d->meta.origin.x;
        return object;
    }
    return Object();
}

Object::Id Object::pathId(const Path& path)
{
    return fs::relative(path.first, path.second).generic_string();
}

bool Object::exists(const boost::filesystem::path& path)
{
    return fs::exists(path / c::object::METAFILE);
}

} // namespace pt
