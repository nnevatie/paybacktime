#pragma once

#include <vector>

#include <glm/common.hpp>

#include "img/image.h"
#include "img/color.h"
#include "size.h"

namespace pt
{

template <typename T>
struct Grid
{
    Grid()
    {}

    Grid(const Size<int>& size) :
        size(size), data(size.h * size.w)
    {}

    const T& at(int x, int y) const
    {
        return data.at(y * size.w + x);
    }

    T& at(int x, int y)
    {
        return data.at(y * size.w + x);
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
        return &data.at(y * size.w);
    }

    T* ptr(int y)
    {
        return &data.at(y * size.w);
    }

    Grid& operator=(const T& scalar)
    {
        std::fill(data.begin(), data.end(), scalar);
        return *this;
    }

    Size<int>       size;
    std::vector<T>  data;
};

inline Image image(const Grid<float>& grid, float sat = 1.f)
{
    Image img(grid.size, 1);
    for (int y = 0; y < grid.size.h; ++y)
        for (int x = 0; x < grid.size.w; ++x)
            *img.bits<uint8_t>(x, y) = 255.f * grid.at(x, y) / sat;
    return img;
}

inline Image image(const Grid<glm::vec3>& grid, float sat = 1.f)
{
    Image img(grid.size, 4);
    for (int y = 0; y < grid.size.h; ++y)
        for (int x = 0; x < grid.size.w; ++x)
            *img.bits<uint32_t>(x, y) =
                argb(255.f * glm::min(glm::vec3(1.f), grid.at(x, y) / sat));
    return img;
}

} // namespace pt
