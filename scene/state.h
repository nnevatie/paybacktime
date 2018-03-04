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
    State();
    State(const json& meta);

    glm::mat4x4 xform() const;

    State& toggle(TimePoint time);
    State& activate(const std::string& name);
    State& animate(TimePoint time, Duration step);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
