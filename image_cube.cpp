#include "image_cube.h"

#include <boost/algorithm/string/replace.hpp>

#include "log.h"
#include "clock.h"

namespace hc
{

ImageCube::ImageCube(const std::string& filename, int depth)
{
    HCTIME("load images");

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
        const auto fn = boost::replace_all_copy(filename, "*", sideImage.name);
        sideImage.image = Image(fn, depth);
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
                    HCLOG(Debug) << __FUNCTION__ << " using " << fallback.name
                                                 << " as "    << sideImages[i].name;
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
    int w0 = std::min(side(Front).rect().w, side(Back).rect().w);
    int w1 = std::min(side(Top).rect().w, side(Bottom).rect().w);
    return std::min(w0, w1);
}

int ImageCube::height() const
{
    int h0 = std::min(side(Front).rect().h, side(Back).rect().h);
    int h1 = std::min(side(Left).rect().h, side(Right).rect().h);
    return std::min(h0, h1);
}

int ImageCube::depth() const
{
    int d0 = std::min(side(Left).rect().w, side(Right).rect().w);
    int d1 = std::min(side(Top).rect().h, side(Bottom).rect().h);
    return std::min(d0, d1);
}

} // namespace
