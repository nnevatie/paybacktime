#include "texture_store.h"

#include "common/log.h"

namespace pt
{

TextureStore::TextureStore(const Size<int>& size) :
    albedo(size, true,  2),
    light(size,  true,  2),
    normal(size, false, 2)
{
    HCLOG(Info) << "size: " << size.w << "x" << size.h;
}

} // namespace pt
