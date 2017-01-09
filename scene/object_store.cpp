#include "object_store.h"

#include <vector>

#include "platform/clock.h"
#include "common/log.h"
#include "constants.h"

namespace pt
{

int createObjects(ObjectStore::Objects& objects,
                  const fs::path& path, TextureStore* textureStore)
{
    int objectCount = 0;
    for (const auto& entry : fs::directory_iterator(path))
        if (fs::is_directory(entry) && Object::exists(entry))
        {
            objects.push_back(Object({entry.path(), path}, textureStore));
            ++objectCount;
        }

    return objectCount;
}

struct ObjectStore::Data
{
    Objects objects;

    Data()
    {}
};

ObjectStore::ObjectStore(const fs::path& path, TextureStore* textureStore) :
    d(std::make_shared<Data>())
{
    PTTIME("create objects");
    const auto objectCount = createObjects(d->objects, path, textureStore);
    PTLOG(Info) << std::to_string(objectCount) + " objects";
}

ObjectStore::Objects ObjectStore::objects() const
{
    return d->objects;
}

Object ObjectStore::object(int index) const
{
    return index >= 0 && index < int(d->objects.size()) ?
           d->objects.at(index) : Object();
}

Object ObjectStore::object(const std::string& name) const
{
    for (const auto& obj : d->objects)
        if (obj.name() == name)
            return obj;

    return Object();
}

} // namespace pt
