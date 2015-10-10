#pragma once

#include <sstream>

#include "file_system.h"

#define HCSOURCE() \
    hc::SourceLocation(__FILE__, __FUNCTION__, __LINE__)

namespace hc
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

std::string readFile(const filesystem::path& path, bool binary = true);

}  // namespace
