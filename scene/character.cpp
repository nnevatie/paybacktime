#include "character.h"

#include "common/log.h"
#include "object.h"

namespace pt
{

struct Character::Data
{
    static const int partCount = 15;

    Data(const fs::path& path, TextureStore* textureStore)
    {
        const std::string partDirs[partCount] =
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
        for (int i = 0; i < partCount; ++i)
        {
            const fs::path partPath(path / partDirs[i]);
            if (fs::exists(partPath))
                parts[i] = Object(partPath, textureStore);
        }
        const int fallbacks[partCount] =
        {
            -1, -1, -1, 4, 3, 6, 5, 8, 7, 10, 9, 12, 11, 14, 13
        };
        for (int i = 0; i < partCount; ++i)
            if (!parts[i])
                parts[i] = parts[fallbacks[i]];
    }

    std::array<Object, partCount> parts;
};

Character::Character()
{}

Character::Character(const fs::path& path, TextureStore* textureStore) :
    d(std::make_shared<Data>(path, textureStore))
{}

const std::array<Object, 15>* Character::parts() const
{
    return &d->parts;
}

} // namespace pt
