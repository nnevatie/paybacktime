#include "state.h"

#include <unordered_map>

namespace pt
{
namespace
{
// State item type
struct StateItem
{
};

// State map
using States = std::unordered_map<std::string, StateItem>;

void setupStates(States& states, const json& meta)
{
}

} // namespace

struct State::Data
{
    Data(const json& meta)
    {}

    States     states;
    StateItem* active;
};

State::State(const json& meta) :
    d(std::make_shared<Data>(meta))
{}

State& State::activate(const std::string& name)
{
    return *this;
}

State& State::animate(TimePoint time, Duration /*step*/)
{
    if (d->active)
    {
    }
    return *this;
}

} // namespace pt
