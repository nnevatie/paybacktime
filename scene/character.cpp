#include "character.h"

#include <ozz/animation/offline/raw_skeleton.h>

#include "common/log.h"
#include "object.h"

namespace pt
{
// Ozz types
using ozz::math::Float3;
using ozz::math::Quaternion;
using ozz::animation::offline::RawSkeleton;

namespace
{

void setupSkeleton(RawSkeleton& skeleton)
{
    {
        // Root
        skeleton.roots.resize(1);
        auto& root = skeleton.roots.front();
        root.name = "root";
        auto& transform = root.transform;
        transform.scale       = Float3::one();
        transform.translation = Float3(0.f, 0.f, 0.f);
        transform.rotation    = Quaternion::identity();

        // TODO: read the skeleton data from character.json
    }
}

} // namespace

struct Character::Data
{
    static constexpr int PART_COUNT = 15;
    using Parts = std::array<Object, PART_COUNT>;

    Data(const fs::path& path, TextureStore& textureStore)
    {
        const std::string partDirs[PART_COUNT] =
        {
            "head",
            "torso",
            "waist",
            "thigh.left",
            "thigh.right",
            "leg.left",
            "leg.right",
            "foot.left",
            "foot.right",
            "arm.left",
            "arm.right",
            "forearm.left",
            "forearm.right",
            "hand.left",
            "hand.right"
        };
        for (int i = 0; i < PART_COUNT; ++i)
        {
            const fs::path partPath(path / partDirs[i]);
            if (fs::exists(partPath))
                parts[i] = Object({partPath, path}, {}, textureStore);
        }
        const int fallbacks[PART_COUNT] =
        {
            -1, -1, -1, 4, 3, 6, 5, 8, 7, 10, 9, 12, 11, 14, 13
        };
        for (int i = 0; i < PART_COUNT; ++i)
            if (!parts[i]) parts[i] = parts[fallbacks[i]].flipped(textureStore);

        setupSkeleton(skeleton);
    }

    Parts       parts;
    RawSkeleton skeleton;
};

Character::Character()
{}

Character::Character(const fs::path& path, TextureStore& textureStore) :
    d(std::make_shared<Data>(path, textureStore))
{}

const std::array<Object, 15>* Character::parts() const
{
    return &d->parts;
}

} // namespace pt
