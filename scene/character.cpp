#include "character.h"

#include "common/metadata.h"
#include "common/log.h"

#include "animation.h"
#include "object.h"

#include "constants.h"

namespace pt
{

static const std::string PART_DIRS[Character::PART_COUNT] =
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

static const std::string JOINT_NAMES[Character::PART_COUNT] =
{
    "Head",
    "Spine",
    "Hips",
    "LeftUpLeg",
    "RightUpLeg",
    "LeftLeg",
    "RightLeg",
    "LeftToeBase",
    "RightToeBase",
    "",
    "",
    "",
    "",
    "",
    ""
};

using BoneMap = std::array<int, Character::PART_COUNT>;

namespace
{

Character::Parts readParts(const Character::Path& path,
                           TextureStore& textureStore)
{
    Character::Parts parts;
    for (int i = 0; i < Character::PART_COUNT; ++i)
    {
        const fs::path partPath(path.first / PART_DIRS[i]);
        if (fs::exists(partPath))
            parts[i] = Object({partPath, path.first}, {}, textureStore);
    }
    const int fallbacks[Character::PART_COUNT] =
        {-1, -1, -1, 4, 3, 6, 5, 8, 7, 10, 9, 12, 11, 14, 13};

    for (int i = 0; i < Character::PART_COUNT; ++i)
        if (!parts[i]) parts[i] = parts[fallbacks[i]].flipped(textureStore);

    return parts;
}

BoneMap createBoneMap(const Animation& anim)
{
    BoneMap map;
    for (int i = 0; i < Character::PART_COUNT; ++i)
    {
        map[i] = anim.jointIndex(JOINT_NAMES[i]);
        PTLOG(Info) << "'" << JOINT_NAMES[i] << "' -> " << map[i];
    }
    return map;
}

Character::Bones createBones(const Character::Parts& parts,
                             const Animation& anim)
{
    Character::Bones bones;
    const int boneCount = bones.size();
    for (int i = 0; i < boneCount; ++i)
        bones[i] = {parts[i], {}};

    return bones;
}

struct Meta
{
    Meta(const Character::Path& path) :
        meta(readJson(path.first / c::character::METAFILE))
    {}

    json meta;
};

} // namespace

struct Character::Data
{
    Data(const Path& path, TextureStore& textureStore) :
        meta(path),
        anim(path.first, meta.meta),
        boneMap(createBoneMap(anim)),
        parts(readParts(path, textureStore)),
        bones(createBones(parts, anim))
    {
        anim.animate(TimePoint(), Duration(0));
    }

    Meta      meta;
    Animation anim;
    BoneMap   boneMap;
    Parts     parts;
    Bones     bones;
};

Character::Character()
{}

Character::Character(const Path& path, TextureStore& textureStore) :
    d(std::make_shared<Data>(path, textureStore))
{}

const Character::Parts* Character::parts() const
{
    return &d->parts;
}

const Character::Bones* Character::bones() const
{
    return &d->bones;
}

Character& Character::animate(TimePoint time, Duration step)
{
    d->anim.animate(time, step);
    for (int i = 0; i < PART_COUNT; ++i)
    {
        const auto boneIndex = d->boneMap[i];
        if (boneIndex >= 0)
            d->bones[i].second = d->anim.jointMatrix(boneIndex);
    }
    return *this;
}

} // namespace pt
