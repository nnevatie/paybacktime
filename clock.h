#pragma once

#include <SDL2/SDL.h>

#include "log.h"

namespace hc
{

#define HCTIME(description)    \
    hc::ScopedClock scopedClock_(HCSOURCE(), description)

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
    Clock() : t0_(0), t1_(0)
    {
        start();
    }

    /**
     * Restarts the clock.
     */
    void start()
    {
        t0_ = ticks();
    }

    /**
     * Stops the clock and returns elapsed time (us).
     */
    double stop()
    {
        t1_ = ticks();
        return us();
    }

    double us() const
    {
        return double(t1_ - t0_) * US_IN_S / frequency();
    }

    double ms() const
    {
        return us() / 1000;
    }

    double s() const
    {
        return us() / 1000000;
    }

    /**
     * Returns the clock's frequency.
     */
    static Time frequency()
    {
        return SDL_GetPerformanceFrequency();
    }

    /**
     * Returns the current tick count.
     */
    static Time ticks()
    {
        return SDL_GetPerformanceCounter();
    }

    /**
     * Returns the current time (us).
     */
    static Time now()
    {
        return ticks() * US_IN_S / frequency();
    }

    /**
     * Sleep for given time (ms).
     */
    static void sleep(Time time)
    {
        SDL_Delay((unsigned int) time);
    }

private:

    Time t0_, t1_;
};

struct ScopedClock
{
    inline ScopedClock(const SourceLocation& source, const std::string& message) :
        source_(source),
        message_(message)
    {
    }

    inline ~ScopedClock()
    {
        clock_.stop();
        Logger(Logger::Info, source_)
            << (message_.length() > 0 ? (message_ + " ") : "")
            << clock_.us() << " " << "us";
    }

private:
    Clock clock_;
    SourceLocation source_;
    std::string message_;
};

}  // namespace
