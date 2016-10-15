#include "image_cube.h"

#include <boost/algorithm/string/replace.hpp>

#include "platform/clock.h"
#include "common/log.h"

namespace pt
{

ImageCube::ImageCube()
{}

ImageCube::ImageCube(const fs::path& path, int depth)
{
    HCLOG(Info) << path.string() << ", " << depth;

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
            path.string(), "*", sideImage.name);

        if (fs::exists(fn))
            sideImage.image = side != Side::Top && side != Side::Bottom ?
                              Image(fn, depth).flipped(Image::Axis::X) :
                              Image(fn, depth);
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
    for (const SideImage& sideImage : sideImages)
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

int ImageCube::width() const
{
    int w0 = std::min(side(Side::Front).size().w, side(Side::Back).size().w);
    int w1 = std::min(side(Side::Top).size().w, side(Side::Bottom).size().w);
    return std::min(w0, w1);
}

int ImageCube::height() const
{
    int h0 = std::min(side(Side::Front).size().h, side(Side::Back).size().h);
    int h1 = std::min(side(Side::Left).size().h, side(Side::Right).size().h);
    return std::min(h0, h1);
}

int ImageCube::depth() const
{
    int d0 = std::min(side(Side::Left).size().w, side(Side::Right).size().w);
    int d1 = std::min(side(Side::Top).size().h, side(Side::Bottom).size().h);
    return std::min(d0, d1);
}

bool ImageCube::transparent() const
{
    for (const Image& side : sides)
        if (side.transparent())
            return true;

    return false;
}

ImageCube ImageCube::scaled(const ImageCube& refCube) const
{
    ImageCube imageCube(*this);
    for (int i = 0; i < 6; ++i)
    {
        const Image& i0 = imageCube.sides[i];
        const Image& i1 = refCube.sides[i];
        if (i0.size() != i1.size())
            imageCube.sides[i] = i0 ? i0.scaled(i1.size()) :
                                      Image(i1.size(), i1.depth()).fill(0x0);
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
    for (const auto& side : sides)
        imageCube.sides.push_back(side.normals());

    return imageCube;
}

} // namespace
