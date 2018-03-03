#pragma once

#include <memory>
#include <string>

#include <glm/mat4x4.hpp>

#include "platform/clock.h"
#include "common/json.h"

namespace pt
{

struct State
{
    State(const json& meta);

    State& activate(const std::string& name);
    State& animate(TimePoint time, Duration step);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
