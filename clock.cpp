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

float Clock::stop()
{
    t1_ = ticks();
    return us();
}

float Clock::us() const
{
    return float(t1_ - t0_) * US_IN_S / frequency();
}

float Clock::ms() const
{
    return us() / 1000;
}

float Clock::s() const
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
    SDL_Delay(uint32_t(time));
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
