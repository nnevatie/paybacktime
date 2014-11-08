#pragma once

#include <iostream>

#include "common.h"

#define HCLOG(priority)    \
    hc::Logger(Logger::priority, HCSOURCE())

namespace hc
{

struct Logger
{
    enum Priority
    {
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };

    inline Logger(Priority priority, const SourceLocation& source) :
        priority_(priority)
    {
        std::cout << priorityName(priority) << "|" <<
            source.file_ << ":" << source.line_ << ":" << source.func_ << ": ";
    }

    inline ~Logger()
    {
        std::cout << std::endl;
    }

    template<typename T> inline Logger& operator<<(const T& t)
    {
        std::cout << t;
        return *this;
    }

    inline Logger& operator<<(std::ostream& (*func)(std::ostream&))
    {
        std::cout << func;
        return *this;
    }

private:
    static inline const char* priorityName(Priority priority)
    {
        const char* priorityNames[] =
            {"DEBUG ", "INFO ", "WARN ", "ERROR", "FATAL"};

        return priorityNames[priority];
    };

    Priority priority_;
};

} // namespace
