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

struct ScenePane
{
    ScenePane(platform::Display* display);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
