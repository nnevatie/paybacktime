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
    "neck",
    "chest",
    "abdomen",
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
    "hand.right",
    "weapon"
};

static const std::string JOINT_NAMES[Character::PART_COUNT] =
{
    "Head",
    "Neck1",
    "Spine1",
    "Spine",
    "Hips",
    "LeftUpLeg",
    "RightUpLeg",
    "LeftLeg",
    "RightLeg",
    "LeftToeBase",
    "RightToeBase",
    "LeftArm",
    "RightArm",
    "LeftForeArm",
    "RightForeArm",
    "LeftHand",
    "RightHand",
    "RightHandMiddle1"
};

using BoneMap = std::array<int, Character::PART_COUNT>;

namespace
{

Character::Parts readParts(const fs::path& path,
                           ObjectStore& objectStore,
                           TextureStore& textureStore)
{
    Character::Parts parts;
    for (int i = 0; i < Character::PART_COUNT; ++i)
    {
        const fs::path partPath(path / PART_DIRS[i]);
        parts[i] = objectStore.object(partPath.generic_path().string());
        PTLOG(Info) << "part " << i << ", "
                    << partPath.generic_path().string()
                    << ": " << bool(parts[i]);
    }
    const int fallbacks[Character::PART_COUNT] =
        {-1, -1, -1, -1, -1, 6, 5, 8, 7, 10, 9, 12, 11, 14, 13, 16, 15, -1};

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

Character::Bones createBones(const Character::Parts& parts)
{
    Character::Bones bones;
    const int boneCount = bones.size();
    for (int i = 0; i < boneCount; ++i)
        bones[i] = {parts[i], {}};

    return bones;
}

struct Meta
{
    Meta(const fs::path& path) :
        meta(readJson(path / c::character::METAFILE)),
        animRoot(meta["animation_root"].get<std::string>())
    {}

    json     meta;
    fs::path animRoot;
};

} // namespace

struct Character::Data
{
    Data(const fs::path& path,
         ObjectStore& objectStore,
         TextureStore& textureStore) :
        meta(objectStore.path() / path),
        anim(meta.animRoot, meta.meta),
        boneMap(createBoneMap(anim)),
        parts(readParts(path, objectStore, textureStore)),
        bones(createBones(parts))
    {
        anim.activate("pose");
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

Character::Character(const Id& id,
                     ObjectStore& objectStore,
                     TextureStore& textureStore) :
    d(std::make_shared<Data>(id, objectStore, textureStore))
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
