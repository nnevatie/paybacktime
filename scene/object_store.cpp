#include "object_store.h"

#include <vector>

#include "platform/clock.h"
#include "common/log.h"
#include "constants.h"

namespace pt
{

int createObjects(ObjectStore::Objects& objects,
                  const Object::Path& path, TextureStore* textureStore)
{
    int objectCount = 0;
    for (const auto& entry : fs::directory_iterator(path.first))
        if (fs::is_directory(entry))
        {
            if (Object::exists(entry))
            {
                objects.push_back(Object({entry.path(), path.second},
                                         textureStore));
                ++objectCount;
            }
            else
            {
                // Recurse into subdir
                objectCount += createObjects(objects, {entry, path.second},
                                             textureStore);
            }
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
    const auto objectCount = createObjects(d->objects, {path, path}, textureStore);
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

ObjectStore& ObjectStore::update(TextureStore* textureStore)
{
    for (auto& object : d->objects)
         object.update(textureStore);

    return *this;
}

} // namespace pt
