#pragma once

#include <ostream>

namespace hc
{

template<typename T>
struct Rect
{
    Rect()
    {}

    Rect(T x, T y, T w, T h) :
        x(x), y(y), w(w), h(h)
    {}

    T x = {}, y = {}, w = {}, h = {};
};

template<typename T>
std::ostream& operator<<(std::ostream& stream, const Rect<T>& rect)
{
    stream << "Rect[x: " << rect.x
           << ", y: " << rect.y
           << ", w: " << rect.w
           << ", h: " << rect.h << "]";
    return stream;
}

} // namespace
