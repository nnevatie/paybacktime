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

    TimeScope() : time(nullptr) {}
    TimeScope(TimeT* time) : time(time) {}

    ~TimeScope()
    {
        end();
    }
    TimeScope& end()
    {
        if (time) time->mark();
        return *this;
    }

private:
    TimeT* time;
};

template <typename T>
struct TimeTree
{
    using Key     = std::string;
    using SubTime = Time<T>;
    using Map     = std::unordered_map<Key, SubTime>;

    TimeTree() {}

    const SubTime& at(const Key& key) const
    {
        return times.at(key);
    }
    SubTime& operator[](const Key& key)
    {
        return times[key];
    }
    TimeScope<T> scope(const Key& key, bool enabled = true)
    {
        return enabled ? TimeScope<T>(&operator[](key)) : TimeScope<T>();
    }
    const Map& map() const
    {
        return times;
    }

private:
    Map times;
};

template <typename T>
struct Throughput
{
    Throughput(float interval = 1.f) :
        interval(interval),
        elapsed(now()),
        value(0.f),
        count(0)
    {}

    float operator()()
    {
        ++count;
        const float t = now();
        const float d = t - elapsed;
        if (d >= interval)
        {
            value    = count / interval;
            count    = 0;
            elapsed += d;
        }
        return value;
    }

private:
    float now() const
    {
        return boost::chrono::duration<float>(time.elapsed()).count();
    }

    Time<T> time;
    float   interval,
            elapsed,
            value;

    int     count;
};
using ThroughputCpu = Throughput<ChronoClock>;

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
