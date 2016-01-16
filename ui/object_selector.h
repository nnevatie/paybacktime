#pragma once

#include <memory>

namespace pt
{
namespace platform
{
struct Display;
}

namespace ui
{

struct ObjectSelector
{
    ObjectSelector(platform::Display* display);

    ObjectSelector& operator()();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
