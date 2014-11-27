#include "clock.h"

#include <SDL2/SDL.h>

#include "log.h"

namespace hc
{

Clock::Clock() : t0_(0), t1_(0)
{
    start();
}

void Clock::start()
{
    t0_ = ticks();
}

double Clock::stop()
{
    t1_ = ticks();
    return us();
}

double Clock::us() const
{
    return double(t1_ - t0_) * US_IN_S / frequency();
}

double Clock::ms() const
{
    return us() / 1000;
}

double Clock::s() const
{
    return us() / 1000000;
}

Clock::Time Clock::frequency()
{
    return SDL_GetPerformanceFrequency();
}

Clock::Time Clock::ticks()
{
    return SDL_GetPerformanceCounter();
}

Clock::Time Clock::now()
{
    return ticks() * US_IN_S / frequency();
}

void Clock::sleep(Clock::Time time)
{
    SDL_Delay((unsigned int) time);
}

ScopedClock::ScopedClock(const SourceLocation& source,
                         const std::string& message) :
    source_(source),
    message_(message)
{
}

ScopedClock::~ScopedClock()
{
    clock_.stop();
    Logger(Logger::Info, source_)
        << (message_.length() > 0 ? (message_ + " ") : "")
        << clock_.us() << " " << "us";
}

}  // namespace
