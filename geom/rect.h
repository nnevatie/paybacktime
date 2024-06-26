#pragma once

#include <ostream>

#include <boost/algorithm/clamp.hpp>

#include <glm/vec2.hpp>

#include "size.h"

namespace pt
{

template<typename T>
struct Rect
{
    Rect()
    {}

    Rect(T x, T y, T w, T h) :
        x(x), y(y), size(w, h)
    {
    }

    Rect(T x, T y, const Size<T>& size) :
        x(x), y(y), size(size)
    {}

    Rect(const Size<T>& size) :
        x(0), y(0), size(size)
    {}

    glm::vec2 point(float fx, float fy) const
    {
        using namespace boost::algorithm;
        const float cfx = clamp(fx, 0.f, 1.f);
        const float cfy = clamp(fy, 0.f, 1.f);
        return {x + cfx * size.w, y + cfy * size.h};
    }

    Rect rect(float fx, float fy, float fw, float fh) const
    {
        using namespace boost::algorithm;
        const float cfx = clamp(fx, 0.f, 1.f);
        const float cfy = clamp(fy, 0.f, 1.f);
        const float cfw = clamp(fw, 0.f, 1.f - cfx);
        const float cfh = clamp(fh, 0.f, 1.f - cfy);
        return Rect(x + cfx * size.w,
                    y + cfy * size.h,
                    cfw * size.w,
                    cfh * size.h);
    }

    Rect translated(T tx, T ty) const
    {
        return Rect(x + tx, y + ty, size.w, size.h);
    }

    Rect scaled(T sx, T sy) const
    {
        return Rect(x * sx, y * sy, size.w * sx, size.h * sy);
    }

    Rect extended(T ex, T ey) const
    {
        return Rect(x - ex, y - ey, size.w + 2 * ex , size.h + 2 * ey);
    }

    Rect transposed() const
    {
        return Rect(y, x, size.h, size.w);
    }

    T area() const
    {
        return size.area();
    }

    template <typename CT>
    CT as() const
    {
        return CT(x, y, size.w, size.h);
    }

    bool operator==(const Rect<T>& other) const
    {
        return x == other.x && y == other.y && size == other.size;
    }
    bool operator!=(const Rect<T>& other) const
    {
        return !operator==(other);
    }

    T x {}, y {};
    Size<T> size {};
};

template<typename T>
std::ostream& operator<<(std::ostream& stream, const Rect<T>& rect)
{
    stream << "Rect[x: " << rect.x
           << ", y: " << rect.y
           << ", w: " << rect.size.w
           << ", h: " << rect.size.h << "]";
    return stream;
}

} // namespace
