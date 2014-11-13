#pragma once

#include <sstream>

#define HCSOURCE() \
    hc::SourceLocation(__FILE__, __FUNCTION__, __LINE__)

namespace hc
{

struct SourceLocation
{
    inline SourceLocation(const char* file, const char* func, int line) :
        file_(file), func_(func), line_(line)
    {
    }

    const char* file_;
    const char* func_;
    const int   line_;
};


std::string str(const std::ostream& ostr)
{
    return static_cast<const std::ostringstream&>(ostr).str();
}

}  // namespace
