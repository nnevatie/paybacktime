#include "mesh.h"

#include "geometry.h"

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

Mesh gridMesh(float interval, float halfWidth, float halfHeight)
{
    const Size<int>   size(halfWidth * 2 / interval, halfHeight * 2 / interval);
    const Size<float> step(halfWidth * 2 / size.w,   halfHeight * 2 / size.h);

    Mesh mesh;
    for (int y = 0; y <= size.h; ++y)
        for (int x = 0; x <= size.w; ++x)
        {
            if (x < size.w)
            {
                const Mesh::Index index0 = mesh.vertices.size();
                mesh.indices.insert(mesh.indices.end(), {index0, index0 + 1});
                mesh.vertices.insert(mesh.vertices.end(),
                    {{-halfWidth  + (x + 0) * step.w, 0,
                      -halfHeight + (y + 0) * step.h},
                     {-halfWidth  + (x + 1) * step.w, 0,
                      -halfHeight + (y + 0) * step.h}});
            }
            if (y < size.h)
            {
                const Mesh::Index index1 = mesh.vertices.size();
                mesh.indices.insert(mesh.indices.end(), {index1, index1 + 1});
                mesh.vertices.insert(mesh.vertices.end(),
                    {{-halfWidth  + (x + 0) * step.w, 0,
                      -halfHeight + (y + 0) * step.h},
                     {-halfWidth  + (x + 0) * step.w, 0,
                      -halfHeight + (y + 1) * step.h}});
            }
        }

    return mesh;
}

} // namespace
