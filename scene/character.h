#pragma once

#include <memory>
#include <string>
#include <utility>

#include <glm/mat4x4.hpp>

#include "platform/clock.h"
#include "common/file_system.h"

#include "object_store.h"
#include "texture_store.h"

#include "object.h"

namespace pt
{

struct Character
{
    // Parts
    static constexpr int PART_COUNT = 18;
    enum class Part
    {
        Head,
        Neck,
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
        HandRight,
        Weapon
    };

    // Types
    using Id    = std::string;
    using Bone  = std::pair<Object, glm::mat4x4>;
    using Parts = std::array<Object, PART_COUNT>;
    using Bones = std::array<Bone, PART_COUNT>;

    Character() = default;
    Character(const Id& id,
              ObjectStore& objectStore,
              TextureStore& textureStore);

    const Parts* parts() const;
    const Bones* bones() const;

    Object volume() const;
    Character& updateVolume();

    Character& activate(const std::string& name);
    Character& animate(TimePoint time, Duration step);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
