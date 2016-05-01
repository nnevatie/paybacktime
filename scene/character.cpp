#include "character.h"

namespace pt
{

struct Character::Data
{
    Data(const fs::path& path, TextureStore* textureStore)
    {}
};

Character::Character()
{}

Character::Character(const fs::path& path, TextureStore* textureStore) :
    d(std::make_shared<Data>(path, textureStore))
{}

} // namespace pt
