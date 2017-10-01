#include "image_cube.h"

#include <unordered_set>

#include <boost/algorithm/string/replace.hpp>

#include "platform/clock.h"
#include "common/log.h"

namespace pt
{

ImageCube::ImageCube()
{}

ImageCube::ImageCube(const fs::path& path, int depth, bool fallback)
{
    struct SideImage
    {
        std::string name;
        Image image;
    }
    sideImages[] = {
        {"front",  {}},
        {"back",   {}},
        {"left",   {}},
        {"right",  {}},
        {"top",    {}},
        {"bottom", {}}
    };

    // Load images
    for (int i = 0; i < 6; ++i)
    {
        const auto side      = Side(i);
        SideImage& sideImage = sideImages[i];

        const auto fn = boost::replace_all_copy(
            path.generic_string(), "*", sideImage.name);

        if (fs::exists(fn))
        {
            sideImage.image = side != Side::Top && side != Side::Bottom ?
                              Image(fn, depth).flipped(Image::Axis::X) :
                              Image(fn, depth);

            const auto imageDepth = sideImage.image.depth();
            if (sideImage.image.depth() != depth)
                throw std::runtime_error("Unexpected depth: " +
                                         std::to_string(imageDepth) +
                                         " != " + std::to_string(depth));
        }
    }

    // Mirror missing images with priority ordered fallbacks
    int fallbacks[6][5] = {
        {1, 2, 3, 4, 5}, // front
        {0, 2, 3, 4, 5}, // back
        {3, 0, 1, 4, 5}, // left
        {2, 0, 1, 4, 5}, // right
        {5, 2, 3, 0, 1}, // top
        {4, 2, 3, 0, 1}  // bottom
    };

    // Find fallbacks
    // TODO: Clone images
    const Image imageDefault(Image(Size<int>(2, 2), depth).fill(0x00));
    if (fallback)
        for (int i = 0; i < 6; ++i)
            if (!sideImages[i].image)
            {
                for (int f = 0; f < 5; ++f)
                {
                    const SideImage& fallback = sideImages[fallbacks[i][f]];
                    if (fallback.image)
                    {
                        sideImages[i].image = fallback.image;
                        break;
                    }
                }
                // Finally, use the default image if all fallbacks failed
                if (!sideImages[i].image)
                    sideImages[i].image = imageDefault;
            }

    // Copy images
    for (const auto& sideImage : sideImages)
        sides.push_back(sideImage.image);
}

ImageCube::operator bool() const
{
    return sides.size() == 6;
}

const Image& ImageCube::side(ImageCube::Side s) const
{
    return sides[int(s)];
}

SizeCube<int> ImageCube::sideSizes() const
{
    return {sides[0].size(), sides[1].size(), sides[2].size(),
            sides[3].size(), sides[4].size(), sides[5].size()};
}

int ImageCube::width() const
{
    auto w0 = std::min(side(Side::Front).size().w, side(Side::Back).size().w);
    auto w1 = std::min(side(Side::Top).size().w, side(Side::Bottom).size().w);
    return std::min(w0, w1);
}

int ImageCube::height() const
{
    auto h0 = std::min(side(Side::Front).size().h, side(Side::Back).size().h);
    auto h1 = std::min(side(Side::Left).size().h, side(Side::Right).size().h);
    return std::min(h0, h1);
}

int ImageCube::depth() const
{
    auto d0 = std::min(side(Side::Left).size().w, side(Side::Right).size().w);
    auto d1 = std::min(side(Side::Top).size().h, side(Side::Bottom).size().h);
    return std::min(d0, d1);
}

bool ImageCube::transparent() const
{
    for (const auto& side : sides)
        if (side.channelPopulated(3, 0xff))
            return true;

    return false;
}

bool ImageCube::emissive() const
{
    for (const auto& side : sides)
        if (side.channelPopulated(2))
            return true;

    return false;
}

ImageCube ImageCube::scaled(const ImageCube& refCube) const
{
    ImageCube imageCube;
    imageCube.sides.resize(6);

    for (int i = 0; i < 6; ++i)
    {
        const Image& i0 = sides[i];
        const Image& i1 = refCube.sides[i];
        if (i0 && i1)
            imageCube.sides[i] = i0.scaled(i1.size());
    }
    return imageCube;
}

ImageCube ImageCube::flipped() const
{
    ImageCube cube(*this);
    std::swap(cube.sides[int(Side::Left)], cube.sides[int(Side::Right)]);

    cube.sides[int(Side::Front)]  = cube.sides[int(Side::Front)].
                                    flipped(Image::Axis::Y);
    cube.sides[int(Side::Back)]   = cube.sides[int(Side::Back)].
                                    flipped(Image::Axis::Y);
    cube.sides[int(Side::Top)]    = cube.sides[int(Side::Top)].
                                    flipped(Image::Axis::Y);
    cube.sides[int(Side::Bottom)] = cube.sides[int(Side::Bottom)].
                                    flipped(Image::Axis::Y);
    return cube;
}

ImageCube ImageCube::normals() const
{
    ImageCube imageCube;
    imageCube.sides.resize(6);

    for (int i = 0; i < 6; ++i)
        if (sides[i]) imageCube.sides[i] = sides[i].normals();

    return imageCube;
}

int ImageCube::merge(const ImageCube& cube)
{
    int mergeCount = 0;
    for (int i = 0; i < 6; ++i)
        if (const auto& side = cube.sides.at(i))
        {
            sides[i] = side.clone();
            ++mergeCount;
        }

    return mergeCount;
}

const ImageCube& ImageCube::validate() const
{
    // Validate depth
    std::unordered_set<int> depths;
    for (const auto& side : sides)
        depths.insert(side.depth());

    if (depths.size() > 1)
        throw std::runtime_error("Mismatching depths");

    return *this;
}

} // namespace
