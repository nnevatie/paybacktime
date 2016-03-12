#include "model.h"

#include <functional>

#include <glm/glm.hpp>
#include <glm/vec2.hpp>

#include "common/log.h"
#include "geom/image_mesher.h"
#include "geom/volume.h"
#include "img/image_cube.h"
#include "img/color.h"
#include "gl/primitive.h"

namespace pt
{

namespace
{

typedef std::function<glm::ivec2(const glm::vec3&)> Projection;

void accumulateEmission(Image* map, const Projection& p,
                                    const Image& depth,
                                    const Image& albedo,
                                    const Image& light)
{
    auto const exp        = 0.025f;
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

struct Model::Data
{
    ImageCube cubeDepth,
              cubeAlbedo,
              cubeLight;

    gl::TextureAtlas::EntryCube entryAlbedo,
                                entryLight;
    gl::Primitive primitive;
    Image         visibility;
    Image         emission;

    Data(const fs::path& path) :
        cubeDepth((path  / "*.png").string(), 1),
        cubeAlbedo((path / "albedo.*.png").string()),
        cubeLight((path  / "light.*.png").string())
    {
        HCLOG(Info) << path.string();
        cubeLight = cubeLight.scaled(cubeAlbedo);
    }
};

Model::Model(const fs::path& path, TextureStore* textureStore) :
    d(std::make_shared<Data>(path))
{
    d->entryAlbedo = textureStore->albedo.insert(d->cubeAlbedo);
    d->entryLight  = textureStore->light.insert(d->cubeLight);
    auto mesh      = ImageMesher::mesh(d->cubeDepth, d->entryAlbedo.second);
    d->primitive   = gl::Primitive(mesh);

    updateVisibility();
    updateEmission();
}

glm::vec3 Model::dimensions() const
{
    return {d->cubeDepth.width(), d->cubeDepth.height(), d->cubeDepth.depth()};
}

gl::Primitive Model::primitive() const
{
    return d->primitive;
}

Image Model::visibility() const
{
    return d->visibility;
}

Model& Model::updateVisibility()
{
    const auto size = dimensions().xz() / 8.f;
    Image map(Size<int>(size.x, size.y), 1);
    map.fill(0x00000000);

    const Cubefield cfield(d->cubeDepth);

    HCLOG(Info) << cfield.width << ", " << cfield.height << ", " << cfield.depth;

    d->visibility = map;
    return *this;
}

Image Model::emission() const
{
    return d->emission;
}

Model& Model::updateEmission()
{
    const auto size = dimensions().xz() / 8.f;
    Image map(Size<int>(size.x, size.y), 4);
    map.fill(0x00000000);

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
        accumulateEmission(&map, p[i], d->cubeDepth.side(side),
                                       d->cubeAlbedo.side(side),
                                       d->cubeLight.side(side));
    }
    d->emission = map;
    return *this;
}

} // namespace pt
