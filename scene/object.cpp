#include "object.h"

namespace pt
{

Object::Object(const fs::path& path, gl::TextureAtlas* atlas) :
    model(path, atlas)
{
}

} // namespace pt
