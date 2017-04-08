#pragma once

#include <memory>

#include "common/file_system.h"
#include "common/json.h"

namespace pt
{

struct Animation
{
    Animation(const fs::path& path, const json& meta);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
