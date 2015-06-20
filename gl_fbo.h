#pragma once

#include <memory>

namespace hc
{
namespace gl
{

struct Fbo
{
    Fbo();

    Fbo& bind();

    static bool unbind();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace gl
} // namespace hc
