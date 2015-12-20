#pragma once

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

    template <typename AT>
    Size operator/(AT d) const
    {
        return Size(T(w / d), T(h / d));
    }

    T w {}, h {};
};

} // namespace
