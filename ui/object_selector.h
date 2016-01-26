#pragma once

#include <memory>

namespace pt
{
struct ObjectStore;
struct TextureStore;

namespace platform
{
struct Display;
}

namespace ui
{

struct ObjectSelector
{
    ObjectSelector(platform::Display* display,
                   ObjectStore* objectStore,
                   TextureStore* textureStore);

    ObjectSelector& operator()();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
