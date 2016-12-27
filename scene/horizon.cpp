#include "horizon.h"

#include "img/image.h"
#include "gl/texture.h"

namespace pt
{

struct Horizon::Data
{
    Data(const fs::path& path) :
        name(path.stem().string())
    {
        texture.bind().alloc(Image(path));
    }

    std::string name;
    gl::Texture texture;
};

Horizon::Horizon()
{}

Horizon::Horizon(const fs::path& path) :
    d(std::make_shared<Data>(path))
{}

Horizon::operator bool() const
{
    return d.operator bool();
}

Horizon::operator==(const Horizon& other) const
{
    return d == other.d;
}

Horizon::operator!=(const Horizon& other) const
{
    return !operator==(other);
}

std::string Horizon::name() const
{
    return d->name;
}

} // namespace pt
