#include "model.h"

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
        cubeDepth((path  / "*.png").string()),
        cubeAlbedo((path / "albedo.*.png").string()),
        cubeLight((path  / "light.*.png").string())
    {}
};

Model::Model(const fs::path& path, TextureStore* textureStore) :
    d(std::make_shared<Data>(path))
{
    d->entryAlbedo = textureStore->albedo.insert(d->cubeAlbedo);
    d->entryLight  = textureStore->light.insert(d->cubeLight);

    const Mesh_P_N_UV mesh = ImageMesher::mesh(d->cubeDepth,
                                               d->entryAlbedo.second);
    d->primitive = gl::Primitive(mesh);
}

} // namespace pt
