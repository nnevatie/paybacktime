#pragma once

#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

#include <SDL2/SDL.h>

#include "common/common.h"
#include "common/log.h"

namespace pt
{
using ChronoClock = boost::chrono::high_resolution_clock;
using TimePoint   = boost::chrono::time_point<ChronoClock>;
using Duration    = ChronoClock::duration;

template <typename T>
struct Time
{
    Time() : tp0(now())
    {}
    TimePoint now() const
    {
        return impl.now();
    }
    Duration elapsed() const
    {
        return now() - tp0;
    }
    Time& sleep(const Duration& duration)
    {
        boost::this_thread::sleep_for(duration);
        return *this;
    }
private:
    T impl;
    TimePoint tp0;
};

#define PTTIME(description) \
    pt::ScopedClock<ChronoClock> scopedClock_(PTSRC(), description)

template <typename T>
struct ScopedClock
{
    ScopedClock(const SourceLocation& source, const std::string& message) :
        source_(source), message_(message)
    {}

    ~ScopedClock()
    {
        const float us = boost::chrono::duration
                         <float, boost::micro>(clock_.elapsed()).count();

        Logger(Logger::Info, source_)
            << (message_.length() > 0 ? (message_ + " ") : "")
            << us << " " << "us";
    }
private:
    Time<T>        clock_;
    SourceLocation source_;
    std::string    message_;
};

}  // namespace
