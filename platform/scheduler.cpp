#include "scheduler.h"

namespace pt
{

Scheduler::Scheduler(const Duration& timeStep,
                     const Simulation& simulation,
                     const Renderer& renderer,
                     Options options) :
    state(StateStopped),
    timeStep(timeStep),
    simulation(simulation),
    renderer(renderer),
    options(options)
{
}

void Scheduler::start()
{
    state = StateRunning;
    Time<ChronoClock> clock;

    TimePoint timeSim;
    TimePoint timePrev = clock.now();
    Duration  durAcc;

    while (state == StateRunning)
    {
        const TimePoint timeNow = clock.now();
        const Duration durFrame = timeNow - timePrev;

        durAcc   += durFrame;
        timePrev  = timeNow;

        while (durAcc >= timeStep)
        {
            if (!simulation(timeSim, timeStep))
                stop();

            timeSim += timeStep;
            durAcc  -= timeStep;
        }
        renderer(float(durAcc.count()) / timeStep.count());

        if (options & OptionPreserveCpu)
            clock.sleep(std::chrono::milliseconds(1));
    }
    state = StateStopped;
}

void Scheduler::stop()
{
    state = StateStopping;
}

}
