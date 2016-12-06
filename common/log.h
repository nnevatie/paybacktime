#pragma once

#include <iostream>

#include "common.h"

#define HCLOG(priority)    \
    pt::Logger(pt::Logger::priority, PTSRC())

namespace pt
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

    Logger(Priority priority, const SourceLocation& source);
    ~Logger();

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
};

} // namespace
