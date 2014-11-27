#include "log.h"

namespace
{

const char* priorityName(hc::Logger::Priority priority)
{
    const char* priorityNames[] =
        {"DEBUG ", "INFO ", "WARN ", "ERROR", "FATAL"};

    return priorityNames[priority];
}

}

namespace hc
{

Logger::Logger(Priority priority, const SourceLocation& source)
{
    std::cout << priorityName(priority) << "|" <<
        source.file << ":" << source.line << ":" << source.func << ": ";
}

Logger::~Logger()
{
    std::cout << std::endl;
}

} // namespace
