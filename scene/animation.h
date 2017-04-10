#pragma once

#include <memory>

#include "platform/clock.h"
#include "common/file_system.h"
#include "common/json.h"

namespace pt
{

struct Animation
{
    Animation(const fs::path& path, const json& meta);

    Animation& animate(TimePoint time, Duration step);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
