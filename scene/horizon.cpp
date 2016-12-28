#include "horizon.h"

namespace pt
{
const Size<int> previewSize(200, 30);

struct Horizon::Data
{
    Data(const Image& image, const std::string& name) :
        name(name),
        image(image),
        preview(image.scaled(previewSize))
    {
        allocTexture();
    }

    Data(const fs::path& path) :
        name(path.stem().string()),
        image(Image(path, 4)),
        preview(image.scaled(previewSize))
    {
        allocTexture();
    }

    Data& allocTexture()
    {
        texture.bind().alloc(image.maxToAlpha())
               .set(GL_TEXTURE_MIN_FILTER, GL_LINEAR)
               .set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return *this;
    }

    std::string name;
    Image       image,
                preview;
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

bool Horizon::operator==(const Horizon& other) const
{
    return d == other.d;
}

bool Horizon::operator!=(const Horizon& other) const
{
    return !operator==(other);
}

std::string Horizon::name() const
{
    return d->name;
}

Image Horizon::image() const
{
    return d->image;
}

Image Horizon::preview() const
{
    return d->preview;
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
