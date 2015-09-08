#pragma once

#include <ostream>

namespace hc
{

template<typename T>
struct Size
{
    Size()
    {}

    Size(T w, T h) :
        w(w), h(h)
    {}

    operator bool() const
    {
        return w && h;
    }

    template <typename CT>
    CT as() const
    {
        return CT {w, h};
    }

    T w {}, h {};
};

template<typename T>
struct Rect
{
    Rect()
    {}

    Rect(T x, T y, T w, T h) :
        x(x), y(y), size(w, h)
    {}

    Rect(T x, T y, const Size<T>& size) :
        x(x), y(y), size(size)
    {}

    Rect(const Size<T>& size) :
        x(0), y(0), size(size)
    {}

    T x {}, y {}, size {};
};

template<typename T>
std::ostream& operator<<(std::ostream& stream, const Size<T>& rect)
{
    stream << "Rect[x: " << rect.x
           << ", y: " << rect.y
           << ", w: " << rect.w
           << ", h: " << rect.h << "]";
    return stream;
}

} // namespace
