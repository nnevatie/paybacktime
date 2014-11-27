#pragma once

#include "common.h"

namespace hc
{

struct Clock
{
    /**
     * Time/frequency/duration data types.
     */
    typedef long long Time;

    /**
     * Conversion constants.
     */
    static const Time US_IN_S = 1000000;

    /**
     * Constructs a clock.
     */
    Clock();

    /**
     * Restarts the clock.
     */
    void start();

    /**
     * Stops the clock and returns elapsed time (us).
     */
    double stop();

    double us() const;
    double ms() const;
    double s() const;

    /**
     * Returns the clock's frequency.
     */
    static Time frequency();

    /**
     * Returns the current tick count.
     */
    static Time ticks();

    /**
     * Returns the current time (us).
     */
    static Time now();
    /**
     * Sleep for given time (ms).
     */
    static void sleep(Time time);

private:

    Time t0_, t1_;
};

#define HCTIME(description)    \
    hc::ScopedClock scopedClock_(HCSOURCE(), description)

struct ScopedClock
{
    ScopedClock(const SourceLocation& source, const std::string& message);
    ~ScopedClock();

private:
    Clock clock_;
    SourceLocation source_;
    std::string message_;
};

}  // namespace
