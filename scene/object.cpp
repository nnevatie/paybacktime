#include "object.h"

#include "common/log.h"

namespace pt
{

Object::Object(const fs::path& path, TextureStore* textureStore) :
    model(path, textureStore)
{
    // TODO: Parse json properties
    name = path.filename().string();
}

} // namespace pt
