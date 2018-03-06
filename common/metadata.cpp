#include "metadata.h"

#include "common.h"
#include "log.h"

namespace pt
{

json readJson(const fs::path& path)
{
    try
    {
        return fs::exists(path) ?
               json::parse(readFile(path)) :
               json();
    }
    catch (const std::exception& e)
    {
        PTLOG(Warn) << e.what();
        return {};
    }
}

} // namespace pt
