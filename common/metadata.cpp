#include "metadata.h"

#include "common.h"
#include "log.h"

namespace pt
{

json readJson(const fs::path& path)
{
    try
    {
        if (fs::exists(path))
        {
            const auto buf = readFile(path);
            return buf.size() > 0 ? json::parse(buf) : json();
        }
    }
    catch (const std::exception& e)
    {
        PTLOG(Warn) << e.what();
    }
    return {};
}

} // namespace pt
