#pragma once

#include <boost/filesystem.hpp>

namespace pt
{
namespace fs = boost::filesystem;

static std::time_t lastModified(const fs::path& path)
{
    std::time_t time = 0;
    for (const auto& entry : fs::directory_iterator(path))
        if (fs::is_directory(entry))
            time = std::max(time, lastModified(entry));
        else
            time = std::max(time, fs::last_write_time(entry));

    return time;
}

} // namespace
