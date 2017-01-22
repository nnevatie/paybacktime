#pragma once

#include <unordered_map>

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
    Time() : tp0(now()), tp1(tp0)
    {}
    TimePoint now() const
    {
        return impl.now();
    }
    Time& mark()
    {
        tp1 = now();
        return *this;
    }
    Duration duration() const
    {
        return tp1 - tp0;
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
    TimePoint tp0, tp1;
};

template <typename T>
struct TimeScope
{
    using TimeT = Time<T>;

    TimeScope(TimeT& time) : time(time)
    {}
    ~TimeScope()
    {
        time.mark();
    }
    TimeScope& end()
    {
        time.mark();
        return *this;
    }

private:
    TimeT& time;
};

template <typename T>
struct TimeTree
{
    using Key     = std::string;
    using SubTime = Time<T>;

    TimeTree() {}

    const SubTime& at(const Key& key) const
    {
        return times.at(key);
    }
    SubTime& operator[](const Key& key)
    {
        return times[key];
    }
    TimeScope<T> scope(const Key& key)
    {
        return TimeScope<T>(operator[](key));
    }

private:
    std::unordered_map<Key, SubTime> times;
};

#define PTTIME(description) \
    pt::ScopedClock<ChronoClock> scopedClock_(PTSRC(), description)

#define PTTIMEU(description, unit) \
    pt::ScopedClock<ChronoClock, unit> scopedClock_(PTSRC(), description)

template <typename T>
inline std::string durationSuffix() {return {};}
template <>
inline std::string durationSuffix<boost::milli>() {return "ms";}
template <>
inline std::string durationSuffix<boost::micro>() {return "us";}

template <typename T, typename U = boost::micro>
struct ScopedClock
{
    ScopedClock(const SourceLocation& source, const std::string& message) :
        source_(source), message_(message)
    {}

    ~ScopedClock()
    {
        const float us = boost::chrono::duration
                         <float, U>(clock_.elapsed()).count();

        Logger(Logger::Info, source_)
            << (message_.length() > 0 ? (message_ + " ") : "")
            << us << " " << durationSuffix<U>();
    }

private:
    Time<T>        clock_;
    SourceLocation source_;
    std::string    message_;
};

}  // namespace
