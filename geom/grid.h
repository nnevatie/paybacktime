#pragma once

#include <vector>

#include <glm/common.hpp>
#include <glm/vec3.hpp>

#include "img/image.h"
#include "img/color.h"

#include "size.h"
#include "aabb.h"

namespace pt
{

template <typename T>
struct Grid
{
    Grid() = default;

    Grid(const Size<int>& size) :
        size(size.w, size.h, 1), data(size.w * size.h)
    {}

    Grid(const glm::ivec3& size) :
        size(size), data(size.x * size.y * size.z)
    {}

    Aabb bounds(const glm::vec3& pos, const glm::vec3& origin) const
    {
        const auto dim  = glm::vec3(size);
        const auto hdim = 0.5f * glm::vec3(dim.x, dim.y, 0.f);
        const auto min  = pos - origin - hdim;
        const auto max  = min + dim;
        #if 0
        PTLOG(Info) << "dim: " << glm::to_string(dim)
                    << ", pos: " << glm::to_string(pos)
                    << ", origin: " << glm::to_string(origin);
        #endif
        return {min, max};
    }

    std::vector<int> dims() const
    {
        std::vector<int> d;
        if (const int dimc = (size.x > 0) + (size.y > 0) + (size.z > 0))
            d.insert(d.begin(), &size[0], &size[dimc - 1] + 1);
        return d;
    }

    inline const T& at(int x, int y) const
    {
        return data.at(y * size.x + x);
    }

    inline const T& at(int x, int y, int z) const
    {
        return data.at(z * size.x * size.y + y * size.x + x);
    }

    inline T& at(int x, int y)
    {
        return data.at(y * size.x + x);
    }

    inline T& at(int x, int y, int z)
    {
        return data.at(z * size.x * size.y + y * size.x + x);
    }

    inline const T& at(const glm::ivec3& v) const
    {
        return at(v.x, v.y, v.z);
    }

    inline T& at(const glm::ivec3& v)
    {
        return at(v.x, v.y, v.z);
    }

    const T* ptr() const
    {
        return data.data();
    }

    T* ptr()
    {
        return data.data();
    }

    const T* ptr(int y) const
    {
        return &data.at(y * size.x);
    }

    T* ptr(int y)
    {
        return &data.at(y * size.x);
    }

    Grid& operator=(const T& scalar)
    {
        std::fill(data.begin(), data.end(), scalar);
        return *this;
    }

    operator bool() const
    {
        return !data.empty();
    }

    glm::ivec3     size;
    std::vector<T> data;
};

inline Image image(const Grid<float>& grid, int z = 0, float sat = 1.f)
{
    Image img(grid.size.xy(), 1);
    for (int y = 0; y < grid.size.y; ++y)
        for (int x = 0; x < grid.size.x; ++x)
            *img.bits<uint8_t>(x, y) = 255.f * grid.at(x, y, z) / sat;
    return img;
}

template<typename T>
inline Image image(const Grid<T>& grid, int z = 0, float sat = 1.f)
{
    Image img(grid.size.xy(), 4);
    for (int y = 0; y < grid.size.y; ++y)
        for (int x = 0; x < grid.size.x; ++x)
            *img.bits<uint32_t>(x, y) =
                argb(glm::uvec4(255.f * glm::min(T(1.f), grid.at(x, y, z) / sat)));
    return img;
}

} // namespace pt
