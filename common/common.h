#pragma once

#include <sstream>
#include <cmath>
#include <gsl.h>

#include "file_system.h"

#define HCSOURCE() \
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

template <class D, class S>
inline D bitCast(const S& src)
{
  static_assert(sizeof(D) == sizeof(S),
                "Src and dst are required to have same size");

  D dst;
  memcpy(&dst, &src, sizeof(dst));
  return dst;
}

std::string str(const std::ostream& ostr);

std::string readFile(const fs::path& path, bool binary = true);

template <typename T>
inline T radians(T deg)
{
     return T(deg * (M_PI / 180));
}

}  // namespace
