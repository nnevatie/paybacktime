#include "horizon.h"

namespace pt
{

struct Horizon::Data
{
    Data(const Image& image, const std::string& name) :
        name(name)
    {
        texture.bind().alloc(image)
                      .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                      .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    Data(const fs::path& path) :
        name(path.stem().string())
    {
        texture.bind().alloc(Image(path))
                      .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
                      .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    std::string name;
    gl::Texture texture;
};

Horizon::Horizon()
{}

Horizon::Horizon(const Image& image, const std::string& name) :
    d(std::make_shared<Data>(image, name))
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

gl::Texture Horizon::texture() const
{
    return d->texture;
}

Horizon Horizon::none()
{
    return Horizon(Image(Size<int>(2, 2), 4).fill(0x00), "none");
}

} // namespace pt
