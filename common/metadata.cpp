#include "metadata.h"

#include "common.h"
#include "log.h"

namespace pt
{

json readJson(const fs::path& path)
{
    try
    {
        return json::parse(readFile(path, false));
    }
    catch (const std::exception& e)
    {
        return {};
    }
}

} // namespace pt
