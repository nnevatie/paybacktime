#include "image_cube.h"

#include <boost/algorithm/string/replace.hpp>

#include "platform/clock.h"
#include "common/log.h"

namespace pt
{

ImageCube::ImageCube(const fs::path& path, int depth)
{
    HCLOG(Info) << path.string() << ", " << depth;

    struct SideImage
    {
        std::string name;
        Image image;
    }
    sideImages[] = {
        {"front",  Image()},
        {"back",   Image()},
        {"left",   Image()},
        {"right",  Image()},
        {"top",    Image()},
        {"bottom", Image()}
    };

    // Load images
    for (SideImage& sideImage : sideImages)
    {
        const auto fn = boost::replace_all_copy(
            path.string(), "*", sideImage.name);

        sideImage.image = Image(fn, depth).flipped();
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
    for (int i = 0; i < 6; ++i)
        if (!sideImages[i].image)
            for (int f = 0; f < 5; ++f)
            {
                const SideImage& fallback = sideImages[fallbacks[i][f]];
                if (fallback.image)
                {
                    sideImages[i].image = fallback.image;
                    //HCLOG(Debug) << __FUNCTION__ << " using " << fallback.name
                    //                             << " as "    << sideImages[i].name;
                    break;
                }
            }

    // Copy images
    for (const SideImage& sideImage : sideImages)
        sides.push_back(sideImage.image);
}

const Image& ImageCube::side(ImageCube::Side s) const
{
    return sides[s];
}

int ImageCube::width() const
{
    int w0 = std::min(side(Front).size().w, side(Back).size().w);
    int w1 = std::min(side(Top).size().w, side(Bottom).size().w);
    return std::min(w0, w1);
}

int ImageCube::height() const
{
    int h0 = std::min(side(Front).size().h, side(Back).size().h);
    int h1 = std::min(side(Left).size().h, side(Right).size().h);
    return std::min(h0, h1);
}

int ImageCube::depth() const
{
    int d0 = std::min(side(Left).size().w, side(Right).size().w);
    int d1 = std::min(side(Top).size().h, side(Bottom).size().h);
    return std::min(d0, d1);
}

} // namespace
