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
        anim(meta.meta)
    {}

    Meta      meta;
    Parts     parts;
    Animation anim;
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
