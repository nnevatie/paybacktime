#pragma once

#include <memory>

namespace pt
{
struct Object;
struct ObjectStore;
struct TextureStore;

namespace platform
{
struct Display;
}

namespace ui
{

struct ObjectPane
{
    ObjectPane(platform::Display* display,
                   ObjectStore* objectStore,
                   TextureStore* textureStore);

    Object selectedObject() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
