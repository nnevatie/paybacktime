#pragma once

#include <chrono>
#include <boost/thread/thread.hpp>

#include <SDL2/SDL.h>

#include "common/common.h"
#include "common/log.h"

namespace hc
{
using ChronoClock = std::chrono::high_resolution_clock;
using TimePoint   = std::chrono::time_point<ChronoClock>;
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
        boost::chrono::nanoseconds durationBoost(
            std::chrono::duration_cast
                <std::chrono::nanoseconds>(duration).count());

        boost::this_thread::sleep_for(durationBoost);
        return *this;
    }
private:
    T impl;
    TimePoint tp0;
};

#define HCTIME(description) \
    hc::ScopedClock<ChronoClock> scopedClock_(HCSOURCE(), description)

template <typename T>
struct ScopedClock
{
    ScopedClock(const SourceLocation& source, const std::string& message) :
        source_(source), message_(message)
    {}

    ~ScopedClock()
    {
        const float us = std::chrono::duration
                         <float, std::micro>(clock_.elapsed()).count();

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
