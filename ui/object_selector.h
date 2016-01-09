#pragma once

#include <memory>

struct NVGcontext;

namespace pt
{
namespace ui
{

struct ObjectSelector
{
    ObjectSelector(NVGcontext* vg);

    ObjectSelector& operator()();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
