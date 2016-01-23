#include "object.h"

#include "common/log.h"

namespace pt
{

Object::Object(const fs::path& path, TextureStore* textureStore) :
    model(path, textureStore)
{
    const std::string name(path.filename().string());
    HCLOG(Info) << name;
}

} // namespace pt
