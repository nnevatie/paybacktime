#pragma once

#include <memory>

namespace hc
{
namespace platform
{

struct Context
{
    Context();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace platform
} // namespace hc
