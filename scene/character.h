#pragma once

#include <memory>
#include <string>
#include <utility>

#include <glm/mat4x4.hpp>

#include "platform/clock.h"
#include "common/file_system.h"

#include "texture_store.h"

#include "object.h"

namespace pt
{

struct Character
{
    // Parts
    static constexpr int PART_COUNT = 16;
    enum class Part
    {
        Head,
        Chest,
        Abdomen,
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
    using Bone  = std::pair<Object, glm::mat4x4>;
    using Parts = std::array<Object, PART_COUNT>;
    using Bones = std::array<Bone, PART_COUNT>;

    Character();
    Character(const Path& path, TextureStore& textureStore);

    const Parts* parts() const;
    const Bones* bones() const;

    Character& animate(TimePoint time, Duration step);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
