#pragma once

#include <sstream>

#include "file_system.h"

#define HCSOURCE() \
    hc::SourceLocation(__FILE__, __FUNCTION__, __LINE__)

namespace hc
{

struct SourceLocation
{
    SourceLocation(const char* file, const char* func, int line);

    const char* file;
    const char* func;
    const int   line;
};

std::string str(const std::ostream& ostr);

std::string readFile(const filesystem::path& path, bool binary = true);

}  // namespace
