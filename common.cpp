#include "common.h"

namespace hc
{

SourceLocation::SourceLocation(const char* file, const char* func, int line) :
    file(file), func(func), line(line)
{
}

std::string str(const std::ostream& ostr)
{
    return static_cast<const std::ostringstream&>(ostr).str();
}

}  // namespace
