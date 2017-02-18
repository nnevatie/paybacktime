#include "object_store.h"

#include <vector>

#include "platform/clock.h"
#include "common/log.h"
#include "constants.h"

namespace pt
{

struct ObjectStore::Data
{
    Data(const fs::path& path, TextureStore& textureStore) :
        path(path)
    {
        PTTIMEU("create objects", boost::milli);
        const auto objectCount = createObjects({path, path}, *this, textureStore);
        PTLOG(Info) << std::to_string(objectCount) + " objects";
    }

    Object resolve(const Object::Id& id, TextureStore& textureStore)
    {
        const auto index = indexOf(objects, id);
        PTLOG(Info) << "id: " << id << " -> " << index;
        return index >= 0 ? objects.at(index) :
               createObject({path / id, path}, *this, textureStore);
    }

    operator Object::Resolver()
    {
        return std::bind(&Data::resolve, this, std::placeholders::_1,
                                               std::placeholders::_2);
    }

    int indexOf(const ObjectStore::Objects& objects, const Object::Id& id) const
    {
        for (int i = 0, c = int(objects.size()); i < c; ++i)
            if (objects.at(i).id() == id)
                return i;

        return -1;
    }

    int createObjects(const Object::Path& path,
                      const Object::Resolver& resolver,
                      TextureStore& textureStore)
    {
        int objectCount = 0;
        for (const auto& entry : fs::directory_iterator(path.first))
            if (fs::is_directory(entry))
            {
                const Object::Path objPath(entry.path(), path.second);
                if (indexOf(objects, Object::pathId(objPath)) == -1 &&
                    Object::exists(entry))
                {
                    createObject(objPath, resolver, textureStore);
                    ++objectCount;
                }
                else
                    // Recurse into subdir
                    objectCount += createObjects({entry, path.second},
                                                 resolver, textureStore);
            }

        return objectCount;
    }

    Object createObject(const Object::Path& path,
                        const Object::Resolver& resolver,
                        TextureStore& textureStore)
    {
        const Object object(path, resolver, textureStore);
        objects.push_back(object);
        return object;
    }

    fs::path path;
    Objects  objects;
};

ObjectStore::ObjectStore(const fs::path& path, TextureStore& textureStore) :
    d(std::make_shared<Data>(path, textureStore))
{}

ObjectStore::Objects ObjectStore::objects() const
{
    return d->objects;
}

Object ObjectStore::object(int index) const
{
    return index >= 0 && index < int(d->objects.size()) ?
           d->objects.at(index) : Object();
}

Object ObjectStore::object(const Object::Id& id) const
{
    for (const auto& obj : d->objects)
        if (obj.id() == id)
            return obj;

    return Object();
}

int ObjectStore::update(TextureStore& textureStore)
{
    int updateCount = 0;
    for (auto& object : d->objects)
         updateCount += object.update(textureStore);

    return updateCount;
}

} // namespace pt
