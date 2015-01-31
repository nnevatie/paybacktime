#include "image_cube.h"

namespace hc
{

ImageCube::ImageCube(const Sides&& sides) :
    sides(std::move(sides))
{
}

} // namespace
