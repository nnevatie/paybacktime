#pragma once

#include <memory>
#include <string>
#include <utility>

#include "platform/clock.h"
#include "common/file_system.h"

#include "texture_store.h"

#include "object.h"

namespace pt
{

struct Character
{
    // Parts
    static constexpr int PART_COUNT = 15;
    enum class Part
    {
        Head,
        Torso,
        Waist,
        ThighLeft,
        ThighRight,
        LegLeft,
        LegRight,
        FootLeft,
        FootRight,
        ArmLeft,
        ArmRight,
        ForearmLeft,
        ForearmRight,
        HandLeft,
        HandRight
    };

    // Types
    using Id    = std::string;
    using Path  = std::pair<fs::path, fs::path>; // path, root
    using Parts = std::array<Object, PART_COUNT>;

    Character();
    Character(const Path& path, TextureStore& textureStore);

    const Parts* parts() const;

    Character& animate(TimePoint time, Duration step);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
