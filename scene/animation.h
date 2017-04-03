#pragma once

#include <memory>

#include "common/json.h"

namespace pt
{

struct Animation
{
    Animation(const json& meta);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
