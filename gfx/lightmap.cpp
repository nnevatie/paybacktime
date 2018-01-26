#include "lightmapper.h"

namespace pt
{
namespace gfx
{

struct Lightmap::Data
{
    Data()
    {}
};

Lightmap::Lightmap() :
    d(std::make_shared<Data>())
{}

} // namespace gfx
} // namespace pt
