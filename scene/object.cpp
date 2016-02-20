#include "object.h"

#include "common/log.h"

namespace pt
{

struct Object::Data
{
    Data(const fs::path& path, TextureStore* textureStore) :
        model(path, textureStore)
    {
        // TODO: Parse json properties
        name = path.filename().string();
    }

    std::string name;
    Model       model;
};

Object::Object()
{}

Object::Object(const fs::path& path, TextureStore* textureStore) :
    d(std::make_shared<Data>(path, textureStore))
{}

Object::operator bool() const
{
    return d.operator bool();
}

std::string Object::name() const
{
    return d->name;
}

Model Object::model() const
{
    return d->model;
}

} // namespace pt
