#pragma once

#include <sstream>
#include <cmath>
#include <gsl.h>

#include "file_system.h"

#define PTSRC() \
    pt::SourceLocation(__FILE__, __FUNCTION__, __LINE__)

namespace pt
{

struct SourceLocation
{
    SourceLocation(const char* file, const char* func, int line);

    const char* file;
    const char* func;
    const int   line;
};

template <typename T>
struct Binder
{
    explicit Binder(gsl::not_null<T*> obj) : Binder(*obj)
    {}
    explicit Binder(T& obj) : obj(obj)
    {
        obj.bind();
    }
    ~Binder()
    {
        obj.unbind();
    }
    T& obj;
};

template<typename T, std::size_t N>
constexpr std::size_t countof(T const (&)[N]) noexcept
{
    return N;
}

template <class D, class S>
inline D bitCast(const S& src)
{
    static_assert(sizeof(D) == sizeof(S),
                  "Src and dst are required to have same size");
    D dst;
    memcpy(&dst, &src, sizeof(dst));
    return dst;
}

inline int umod(int x, int y)
{
    return ((x < 0) ? ((x % y) + y) : x) % y;
}

std::string str(const std::ostream& ostr);

std::string readFile(const fs::path& path, bool binary = true);

}  // namespace
