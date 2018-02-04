#pragma once

#include <glm/vec2.hpp>

namespace pt
{

template<typename T>
struct Size
{
    Size() = default;

    Size(T w, T h) :
        w(w), h(h)
    {}

    Size(const glm::ivec2& v) :
        w(v.x), h(v.y)
    {}

    operator bool() const
    {
        return w && h;
    }

    template <typename RT>
    RT aspect() const
    {
        return h > 0 ? RT(w) / RT(h) : RT(0);
    }

    Size scaled(T sx, T sy) const
    {
        return Size(w * sx, h * sy);
    }

    T area() const
    {
        return w * h;
    }

    template <typename RT>
    RT as() const
    {
        return RT(w, h);
    }

    template <typename RT>
    RT inv(int d) const
    {
        return RT(d == 0 ? 1.f / w : 0.f, d == 1 ? 1.f / h : 0.f);
    }

    template <typename AT>
    Size operator+(AT d) const
    {
        return Size(T(w + d), T(h + d));
    }

    template <typename AT>
    Size operator*(AT m) const
    {
        return Size(T(w * m), T(h * m));
    }

    template <typename AT>
    Size operator/(AT d) const
    {
        return Size(T(w / d), T(h / d));
    }

    bool operator==(const Size& other) const
    {
        return w == other.w && h == other.h;
    }

    bool operator!=(const Size& other) const
    {
        return !operator==(other);
    }

    T w {}, h {};
};

} // namespace
