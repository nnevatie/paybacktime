#include "mesh.h"

namespace hc
{

Mesh squareMesh(float halfWidth)
{
    return rectMesh(halfWidth, halfWidth);
}

Mesh rectMesh(float halfWidth, float halfHeight)
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
