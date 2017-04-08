#include "animation.h"

#include <unordered_map>

#include "ozz/base/io/archive.h"
#include <ozz/base/maths/soa_float4x4.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/sampling_job.h>
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
using ozz::math::Float4x4;
using ozz::math::Quaternion;
using ozz::math::SoaTransform;
using ozz::animation::Skeleton;
using ozz::animation::Animation;
using ozz::animation::SamplingCache;
using ozz::animation::offline::RawSkeleton;
using ozz::animation::offline::RawAnimation;
using ozz::animation::offline::SkeletonBuilder;
using ozz::animation::offline::AnimationBuilder;

// Animations map
using Animations = std::unordered_map<std::string, ozz::Animation*>;
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

void setupAnimations(ozz::Animations& animations,
                     const fs::path& path, const json& meta)
{
    auto allocator = ozz::memory::default_allocator();
    for (const auto& nameValue : json::iterator_wrapper(meta))
    {
        // Name and value
        auto  name = nameValue.key();
        auto& node = nameValue.value();

        PTLOG(Info) << name << "|" << node;

        ozz::Animation* animation = nullptr;
        if (node.is_string())
        {
            const auto fn = (path / node.get<std::string>()).generic_string();
            ozz::io::File file(fn.c_str(), "rb");
            if (file.opened())
            {
                ozz::io::IArchive archive(&file);
                if (archive.TestTag<ozz::Animation>())
                {
                    animation = allocator->New<ozz::Animation>();
                    archive >> *animation;
                }
            }
        }
        else
        {
            // Raw animation
            ozz::RawAnimation rawAnimation;

            // Runtime animation
            ozz::AnimationBuilder animationBuilder;
            animation = animationBuilder(rawAnimation);
        }
        if (animation)
        {
            // Store by name
            animations[name] = animation;
            PTLOG(Info) << name << ", '"
                        << animation->name() << "', "
                        << animation->duration() << ", "
                        << animation->num_tracks();
        }
    }
}

ozz::Skeleton* createSkeleton(const fs::path& path, const json& meta)
{
    auto allocator   = ozz::memory::default_allocator();
    const auto& node = meta["skeleton"];

    ozz::Skeleton* skeleton = nullptr;
    if (node.is_string())
    {
        const auto fn = (path / node.get<std::string>()).generic_string();
        ozz::io::File file(fn.c_str(), "rb");
        if (file.opened())
        {
            ozz::io::IArchive archive(&file);
            if (archive.TestTag<ozz::Skeleton>())
            {
                skeleton = allocator->New<ozz::Skeleton>();
                archive >> *skeleton;
            }
        }
    }
    else
    {
        // Raw skeleton
        ozz::RawSkeleton rawSkeleton;
        setupJoints(rawSkeleton.roots, node);
        PTLOG(Info) << "joints: " << rawSkeleton.num_joints();

        // Runtime skeleton
        ozz::SkeletonBuilder skeletonBuilder;
        skeleton = skeletonBuilder(rawSkeleton);
    }
    if (skeleton)
    {
        const auto jointCount = skeleton->num_joints();
        const auto jointNames = skeleton->joint_names();

        PTLOG(Info) << "joints: " << jointCount;
        for (int i = 0; i < jointCount; ++i)
            PTLOG(Info) << "joint: '" << jointNames[i] << "'";
    }
    return skeleton;
}

ozz::Animations createAnimations(const fs::path& path, const json& meta)
{
    ozz::Animations animations;
    setupAnimations(animations, path, meta["animations"]);
    return animations;
}

} // namespace

struct Animation::Data
{
    Data(const fs::path& path, const json& meta) :
        skeleton(createSkeleton(path, meta)),
        animations(createAnimations(path, meta))
    {
        // Runtime buffers and cache
        const auto jointCount = skeleton->num_soa_joints();
        auto allocator = ozz::memory::default_allocator();
        locals = allocator->AllocateRange<ozz::SoaTransform>(jointCount);
        models = allocator->AllocateRange<ozz::Float4x4>(jointCount);
        cache  = allocator->New<ozz::SamplingCache>(jointCount);
    }

    ~Data()
    {
        auto allocator = ozz::memory::default_allocator();
        allocator->Deallocate(locals);
        allocator->Deallocate(models);
        allocator->Delete(cache);
        allocator->Delete(skeleton);
        for (auto& animation : animations)
            allocator->Delete(animation.second);
    }

    ozz::Skeleton*                skeleton;
    ozz::Animations               animations;
    ozz::SamplingCache*           cache;
    ozz::Range<ozz::SoaTransform> locals;
    ozz::Range<ozz::Float4x4>     models;

};

Animation::Animation(const fs::path& path, const json& meta) :
    d(std::make_shared<Data>(path, meta))
{}

} // namespace pt
