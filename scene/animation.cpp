#include "animation.h"

#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include "ozz/animation/offline/raw_animation.h"
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/animation/offline/animation_builder.h>

#include "common/metadata.h"
#include "common/json.h"
#include "common/log.h"

// Ozz types
namespace ozz
{
using ozz::math::Float3;
using ozz::math::Quaternion;
using ozz::animation::Skeleton;
using ozz::animation::Animation;
using ozz::animation::offline::RawSkeleton;
using ozz::animation::offline::RawAnimation;
using ozz::animation::offline::SkeletonBuilder;
using ozz::animation::offline::AnimationBuilder;
}

namespace pt
{
namespace
{

void setupJoints(ozz::RawSkeleton::Joint::Children& joints, const json& meta)
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
        ozz::RawSkeleton::Joint joint = {};

        // Name
        joint.name = name.c_str();

        // Transform
        auto  tr          = node.value("tr",  TR_DEFAULT);
        auto  rot         = node.value("rot", ROT_DEFAULT);
        auto& tform       = joint.transform;
        tform.scale       = ozz::Float3::one();
        tform.translation = ozz::Float3(tr[0], tr[1], tr[2]);
        tform.rotation    = ozz::Quaternion::FromAxisAngle(
                                {rot[0], rot[1], rot[2], rot[3]});
        // Children
        setupJoints(joint.children, node.value("children", json::object()));

        // Add to joints
        joints.push_back(joint);
    }
}

ozz::Skeleton* createSkeleton(const json& meta)
{
    // Raw skeleton
    ozz::RawSkeleton rawSkeleton;
    setupJoints(rawSkeleton.roots, meta["skeleton"]);
    PTLOG(Info) << "joints: " << rawSkeleton.num_joints();

    // Runtime skeleton
    ozz::SkeletonBuilder skeletonBuilder;
    return skeletonBuilder(rawSkeleton);
}

ozz::Animation* createAnimation(const json& meta)
{
    // Raw animation
    ozz::RawAnimation rawAnimation;

    // Runtime animation
    ozz::AnimationBuilder animationBuilder;
    return animationBuilder(rawAnimation);
}

} // namespace

struct Animation::Data
{
    Data(const json& meta) :
        skeleton(createSkeleton(meta)),
        animation(createAnimation(meta))
    {}

    ozz::Skeleton*  skeleton;
    ozz::Animation* animation;
};

Animation::Animation(const json& meta) :
    d(std::make_shared<Data>(meta))
{}

} // namespace pt
