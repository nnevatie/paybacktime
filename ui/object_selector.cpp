#include "object_selector.h"

#include <glad/glad.h>
#include <nanovg.h>

namespace pt
{
namespace ui
{

struct ObjectSelector::Data
{
    explicit Data(NVGcontext* vg) :
        vg(vg)
    {
    }
    ~Data()
    {
    }

    NVGcontext* vg;
};

ObjectSelector::ObjectSelector(NVGcontext* vg) :
    d(new Data(vg))
{
}

ObjectSelector& ObjectSelector::operator()()
{
    return *this;
}

} // namespace ui
} // namespace pt
