#include "object_store.h"

#include "platform/clock.h"
#include "common/log.h"

namespace pt
{

ObjectStore::ObjectStore(const filesystem::path& path, gl::TextureAtlas* atlas)
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

} // namespace pt
