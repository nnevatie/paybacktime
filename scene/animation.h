#pragma once

#include <memory>
#include <string>

#include <glm/mat4x4.hpp>

#include "platform/clock.h"
#include "common/file_system.h"
#include "common/json.h"

namespace pt
{

struct Animation
{
    Animation(const fs::path& path, const json& meta);

    int jointIndex(const std::string& name) const;
    glm::mat4x4 jointMatrix(int index) const;

    Animation& animate(TimePoint time, Duration step);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
