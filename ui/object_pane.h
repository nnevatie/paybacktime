#pragma once

#include <memory>

namespace nanogui
{
class Widget;
}

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
    ObjectPane(nanogui::Widget* parent,
               platform::Display* display,
               ObjectStore* objectStore,
               TextureStore* textureStore);

    Object selected() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace ui
} // namespace pt
