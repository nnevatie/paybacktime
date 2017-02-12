#pragma once

#include <memory>

#include "common/file_system.h"

#include "texture_store.h"

#include "object.h"

namespace pt
{

struct Character
{
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

    Character();
    Character(const fs::path& path, TextureStore& textureStore);

    const std::array<Object, 15>* parts() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
