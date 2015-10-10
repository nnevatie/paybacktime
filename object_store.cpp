#include "object_store.h"

#include "clock.h"
#include "log.h"

namespace hc
{

ObjectStore::ObjectStore(const filesystem::path& path, TextureAtlas* atlas)
{
    HCTIME("load objects");

    for (const filesystem::directory_entry& e :
         filesystem::directory_iterator(path))
        HCLOG(Info) << e.path();
}

} // namespace hc
