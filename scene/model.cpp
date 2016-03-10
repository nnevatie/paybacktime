#include "model.h"

#include <functional>

#include <glm/glm.hpp>
#include <glm/vec2.hpp>

#include "common/log.h"
#include "geom/image_mesher.h"
#include "img/image_cube.h"
#include "img/color.h"
#include "gl/primitive.h"

namespace pt
{

namespace
{

typedef std::function<glm::ivec2(glm::ivec2)> Projection;

void accumulateEmission(Image* map, const Projection& p,
                                    const Image& depth,
                                    const Image& albedo,
                                    const Image& light)
{
    auto const exp        = 0.10f;
    auto const sizeOut    = map->size();
    auto const sizeDepth  = depth.size();
    auto const sizeLight  = light.size();
    auto const scaleLight = (sizeLight.as<glm::vec2>() /
                             sizeDepth.as<glm::vec2>()).x;

    for (int y = 0; y < sizeLight.h; ++y)
    {
        const int y1 = y / float(sizeLight.h) * sizeOut.h;

        uint32_t* __restrict__ rowOut =
            reinterpret_cast<uint32_t*>(map->bits(0, y1));

        const uint32_t* __restrict__ rowAlbedo =
            reinterpret_cast<const uint32_t*>(albedo.bits(0, y));

        const uint32_t* __restrict__ rowLight =
            reinterpret_cast<const uint32_t*>(light.bits(0, y));

        for (int x = 0; x < sizeLight.w; ++x)
        {
            const int x1 = x / float(sizeLight.w) * sizeOut.w;
            auto argb0   = glm::vec4(argbTuple(rowOut[x1]));
            auto argb1   = glm::vec4(argbTuple(rowAlbedo[x]));
            auto emis    = (exp / 255) * argbTuple(rowLight[x]).b / scaleLight;
            auto argb2   = glm::uvec4(argb0 + emis * argb1);
            rowOut[x1]   = argb(glm::min(glm::uvec4(255), argb2));
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
        auto side    = ImageCube::Side(i);
        Projection p = [](glm::ivec2 v) {return v;};
        accumulateEmission(&map, p, d->cubeDepth.side(side),
                                    d->cubeAlbedo.side(side),
                                    d->cubeLight.side(side));
    }
    d->emission = map;
    return *this;
}

} // namespace pt
