#include "state.h"

#include <unordered_map>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/string_cast.hpp>

#include "constants.h"

namespace pt
{
namespace
{
using Seconds = boost::chrono::duration<float>;

struct StateItem
{
    using Ptr  = StateItem*;
    using Next = std::pair<std::string, Seconds>;

    glm::mat4x4 xform;
    Next        next;
};

struct Transition
{
    Transition()
    {
        reset();
    }

    operator bool() const
    {
        return state0 && state1;
    }

    Transition& reset()
    {
        state0 = nullptr;
        state1 = nullptr;
        return *this;
    }

    Transition& start(StateItem::Ptr s0, StateItem::Ptr s1,
                      TimePoint t, Seconds d)
    {
        state0   = s0;
        state1   = s1;
        time     = t;
        duration = d;
        xform    = s0->xform;
        return *this;
    }

    float animate(TimePoint t, Duration /*step*/)
    {
        const auto e0 = t - time;
        const auto e1 = boost::chrono::duration_cast<Seconds>(e0);
        const auto f  = e1.count() / duration.count();
        xform = glm::interpolate(state0->xform, state1->xform, f);
        return f;
    }

    StateItem::Ptr state0,
                   state1;
    TimePoint      time;
    Seconds        duration;
    glm::mat4x4    xform;
};

// State map
using States = std::unordered_map<std::string, StateItem>;

States createStates(const json& meta, StateItem*& initial)
{
    States states;
    for (const auto& nameValue : json::iterator_wrapper(meta))
    {
        // Name and value
        auto  name = nameValue.key();
        auto& node = nameValue.value();
        PTLOG(Info) << name << "|" << node;

        const auto pos = glm::make_vec3(node.value(c::object::state::POS,
                                                   std::vector<float>(3)).data());

        const auto nextJson  = node.value("next", json());
        const auto nextState = nextJson.value("state", std::string());
        const auto nextTime  = nextJson.value("time",  0);

        const auto xform     = glm::translate({}, pos);
        const auto next      = StateItem::Next(nextState, nextTime);
        StateItem state      = {xform, next};
        states[name]         = state;

        if (!initial)
            initial = &states[name];
    }
    return states;
}

} // namespace

struct State::Data
{
    Data(const json& meta) :
        active(nullptr),
        states(createStates(meta, active))
    {}

    StateItem::Ptr active;
    States         states;
    Transition     transition;
};

State::State() :
    d(std::make_shared<Data>(json()))
{}

State::State(const json& meta) :
    d(std::make_shared<Data>(meta))
{}

glm::mat4x4 State::xform() const
{
    return d->transition ? d->transition.xform :
           d->active     ? d->active->xform    : glm::mat4x4();
}

State& State::toggle(TimePoint time)
{
    if (d->active && !d->transition)
    {
        const auto& next = d->active->next;
        d->transition.start(d->active, &d->states[next.first],
                            time, next.second);
    }
    return *this;
}

State& State::activate(const std::string& name)
{
    return *this;
}

State& State::animate(TimePoint time, Duration step)
{
    if (d->transition)
        if (d->transition.animate(time, step) >= 1.f)
        {
            d->active = d->transition.state1;
            d->transition.reset();
        }

    return *this;
}

} // namespace pt
