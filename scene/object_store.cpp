#include "object_store.h"

#include <vector>

#include "platform/clock.h"
#include "common/log.h"

namespace pt
{

struct ObjectStore::Data
{
    std::vector<Object> objects;

    Data()
    {}
};

ObjectStore::ObjectStore(const fs::path& path, TextureStore* textureStore) :
    d(std::make_shared<Data>())
{
    HCTIME("create objects");
    int objectCount;
    for (const fs::directory_entry& e : fs::directory_iterator(path))
        if (is_directory(e))
        {
            d->objects.push_back(Object(e.path(), textureStore));
            ++objectCount;
        }
    HCLOG(Info) << std::to_string(objectCount) + " objects";
}

} // namespace pt
