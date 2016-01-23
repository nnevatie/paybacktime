#include "texture_store.h"

#include "common/log.h"

namespace pt
{

TextureStore::TextureStore(const Size<int>& size) :
    albedo(size), light(size)
{
    HCLOG(Info) << "size: " << size.w << "x" << size.h;
}

} // namespace pt
