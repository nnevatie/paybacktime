#include "model.h"

#include "img/image_cube.h"

namespace pt
{

struct Model::Data
{
    ImageCube depth,
              albedo,
              light;

    Data(const fs::path& path) :
        depth((path  / "*.png").string()),
        albedo((path / "albedo.*.png").string()),
        light((path  / "light.*.png").string())
    {}
};

Model::Model(const fs::path& path, gl::TextureAtlas* atlas) :
    d(std::make_shared<Data>(path))
{
}

} // namespace pt
