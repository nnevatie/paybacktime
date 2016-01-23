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

ObjectStore::ObjectStore(const fs::path& path, gl::TextureAtlas* atlas) :
    d(std::make_shared<Data>())
{
    HCTIME("create objects");
    for (const fs::directory_entry& e : fs::directory_iterator(path))
        if (is_directory(e))
        {
            const std::string name(e.path().filename().string());
            HCLOG(Info) << name << " -> " << e.path().string();
            d->objects.push_back(Object(e.path(), atlas));
        }
}

} // namespace pt
