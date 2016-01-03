#pragma once

#include <functional>

#include "clock.h"

namespace hc
{

class Simulation;
class Renderer;

class Scheduler
{
public:

    typedef std::function<bool(TimePoint, Duration)> Simulation;
    typedef std::function<bool(float)> Renderer;

    enum State
    {
        StateStopped,
        StateStopping,
        StateRunning
    };

    enum Options
    {
        OptionNone        = 0x00,
        OptionPreserveCpu = 0x01
    };

    Scheduler(const Duration& timeStep,
              const Simulation& simulation,
              const Renderer& renderer,
              Options options = OptionNone);

    void start();
    void stop();

private:

    State       state;
    Duration    timeStep;
    Simulation  simulation;
    Renderer    renderer;
    Options     options;
};

}
