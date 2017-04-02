#include "character.h"

#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/skeleton_builder.h>

#include "common/metadata.h"
#include "common/json.h"
#include "common/log.h"
#include "object.h"

#include "constants.h"

namespace pt
{
// Ozz types
using ozz::math::Float3;
using ozz::math::Quaternion;
using ozz::animation::Skeleton;
using ozz::animation::offline::RawSkeleton;
using ozz::animation::offline::SkeletonBuilder;

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

void setupJoints(RawSkeleton::Joint::Children& joints, const json& meta)
{
    // Default transformation
    const auto TR_DEFAULT  = std::vector<float> {0.f, 0.f, 0.f};
    const auto ROT_DEFAULT = std::vector<float> {0.f, 1.f, 0.f, 0.f};

    for (const auto& nameValue : json::iterator_wrapper(meta))
    {
        // Name and value
        auto  name = nameValue.key();
        auto& node = nameValue.value();

        PTLOG(Info) << name << "|" << node;

        // Create joint
        RawSkeleton::Joint joint = {};

        // Name
        joint.name = name.c_str();

        // Transform
        auto  tr          = node.value("tr",  TR_DEFAULT);
        auto  rot         = node.value("rot", ROT_DEFAULT);
        auto& tform       = joint.transform;
        tform.scale       = Float3::one();
        tform.translation = Float3(tr[0], tr[1], tr[2]);
        tform.rotation    = Quaternion::FromAxisAngle(
                                {rot[0], rot[1], rot[2], rot[3]});
        // Children
        setupJoints(joint.children, node.value("children", json::object()));

        // Add to joints
        joints.push_back(joint);
    }
}

Skeleton* createSkeleton(const json& meta)
{
    // Raw skeleton
    RawSkeleton rawSkeleton;
    setupJoints(rawSkeleton.roots, meta["skeleton"]);
    PTLOG(Info) << "joints: " << rawSkeleton.num_joints();

    // Runtime skeleton
    SkeletonBuilder skeletonBuilder;
    return skeletonBuilder(rawSkeleton);
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
        parts(readParts(path, textureStore)),
        skeleton(createSkeleton(meta.meta))
    {}

    Meta      meta;
    Parts     parts;
    Skeleton* skeleton;
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

} // namespace pt
