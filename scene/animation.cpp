#include "animation.h"

#include <unordered_map>

#include <glm/gtc/type_ptr.hpp>

#include "ozz/base/io/archive.h"
#include <ozz/base/maths/soa_float4x4.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/local_to_model_job.h>
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
using ozz::animation::SamplingJob;
using ozz::animation::LocalToModelJob;
using ozz::animation::SamplingCache;
using ozz::animation::offline::RawSkeleton;
using ozz::animation::offline::RawAnimation;
using ozz::animation::offline::SkeletonBuilder;
using ozz::animation::offline::AnimationBuilder;
}

namespace pt
{
namespace
{
// Animation item type
struct AnimationItem
{
    ozz::Animation*               anim;
    ozz::SamplingCache*           cache;
    ozz::Range<ozz::SoaTransform> locals;
    ozz::Range<ozz::Float4x4>     models;

    AnimationItem() : anim(nullptr), cache(nullptr)
    {}

    AnimationItem(ozz::Animation*               anim,
                  ozz::SamplingCache*           cache,
                  ozz::Range<ozz::SoaTransform> locals,
                  ozz::Range<ozz::Float4x4>     models) :
        anim(anim), cache(cache), locals(locals), models(models)
    {}

    ~AnimationItem()
    {
        if (anim)
        {
            auto allocator = ozz::memory::default_allocator();
            allocator->Deallocate(locals);
            allocator->Deallocate(models);
            allocator->Delete(cache);
            allocator->Delete(anim);
        }
    }
};
using AnimationItemPtr = std::unique_ptr<AnimationItem>;

// Animations map
using Animations = std::unordered_map<std::string, AnimationItemPtr>;

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

void setupAnimations(Animations& animations,
                     ozz::Skeleton* skeleton,
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
            const auto jointCount    = skeleton->num_joints();
            const auto soaJointCount = skeleton->num_soa_joints();
            animations[name] = std::make_unique<AnimationItem>
            (
                animation,
                allocator->New<ozz::SamplingCache>(jointCount),
                allocator->AllocateRange<ozz::SoaTransform>(soaJointCount),
                allocator->AllocateRange<ozz::Float4x4>(jointCount)
            );
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
        #if 0
        const auto jointCount = skeleton->num_joints();
        const auto jointNames = skeleton->joint_names();
        PTLOG(Info) << "joints: " << jointCount;
        for (int i = 0; i < jointCount; ++i)
            PTLOG(Info) << "joint: '" << jointNames[i] << "'";
        #endif
    }
    return skeleton;
}

Animations createAnimations(ozz::Skeleton* skeleton,
                            const fs::path& path, const json& meta)
{
    Animations animations;
    setupAnimations(animations, skeleton, path, meta["animations"]);
    return animations;
}

} // namespace

struct Animation::Data
{
    Data(const fs::path& path, const json& meta) :
        skeleton(createSkeleton(path, meta)),
        animations(createAnimations(skeleton, path, meta)),
        active(nullptr)
    {}

    ~Data()
    {
        auto allocator = ozz::memory::default_allocator();
        allocator->Delete(skeleton);
    }

    ozz::Skeleton* skeleton;
    Animations     animations;
    AnimationItem* active;
};

Animation::Animation(const fs::path& path, const json& meta) :
    d(std::make_shared<Data>(path, meta))
{}

int Animation::jointIndex(const std::string& name) const
{
    const auto jointCount = d->skeleton->num_joints();
    const auto jointNames = d->skeleton->joint_names();
    for (int i = 0; i < jointCount; ++i)
        if (jointNames[i] == name)
            return i;

    return -1;
}

glm::mat4x4 Animation::jointMatrix(int index) const
{
    if (d->active)
    {
        const auto& model = d->active->models[index];
        return glm::make_mat4(reinterpret_cast<const float*>(&model.cols[0]));
    }
    return {};
}

Animation& Animation::activate(const std::string& name)
{
    for (auto& animation : d->animations)
        if (animation.first == name)
            d->active = animation.second.get();

    return *this;
}

Animation& Animation::animate(TimePoint time, Duration step)
{
    if (d->active)
    {
        const auto t0 = boost::chrono::duration<float>
                       (time.time_since_epoch()).count();

        ozz::SamplingJob     sampJob;
        ozz::LocalToModelJob ltmJob;

              auto item   = d->active;
        const auto  a     = item->anim;
        const auto t1     = std::fmod(t0, a->duration());
        sampJob.animation = a;
        sampJob.cache     = item->cache;
        sampJob.time      = t1;
        sampJob.output    = item->locals;
        sampJob.Run();

        ltmJob.skeleton   = d->skeleton;
        ltmJob.input      = item->locals;
        ltmJob.output     = item->models;
        ltmJob.Run();
    }
    return *this;
}

} // namespace pt
