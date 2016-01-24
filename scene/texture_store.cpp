#include "texture_store.h"

#include "common/log.h"

namespace pt
{

TextureStore::TextureStore(const Size<int>& size) :
    albedo(size, 2), light(size, 2)
{
    HCLOG(Info) << "size: " << size.w << "x" << size.h;
}

} // namespace pt
