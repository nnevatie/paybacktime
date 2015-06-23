#include "geometry.h"

namespace hc
{

Geometry squareGeometry(float halfWidth)
{
    return rectGeometry(halfWidth, halfWidth);
}

Geometry rectGeometry(float halfWidth, float halfHeight)
{
    return
    {
        {
            {-halfWidth, -halfHeight, 0, 0, 0, 1, 0, 0},
            { halfWidth, -halfHeight, 0, 0, 0, 1, 1, 0},
            { halfWidth,  halfHeight, 0, 0, 0, 1, 1, 1},
            {-halfWidth,  halfHeight, 0, 0, 0, 1, 0, 1},
        },
        {0, 1, 2, 2, 3, 0}
    };
}

} // namespace
