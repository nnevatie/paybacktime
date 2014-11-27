#pragma once

#include <sstream>

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

}  // namespace
