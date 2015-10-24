#include "object_store.h"

#include "clock.h"
#include "log.h"

namespace hc
{

ObjectStore::ObjectStore(const filesystem::path& path, TextureAtlas* atlas)
{
    HCTIME("create objects");
    using namespace filesystem;
    for (const directory_entry& e : directory_iterator(path))
        if (e.path().extension() == ".json")
        {
            const std::string name(e.path().stem().string());
            HCLOG(Info) << name << " -> " << e.path().string();
            objects.push_back(Object(e.path(), atlas));
        }
}

} // namespace hc
