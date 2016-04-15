#pragma once

#include <memory>
#include <string>

#include <SDL2/SDL.h>

#include "common/file_system.h"
#include "geom/size.h"

struct NVGcontext;

namespace pt
{

struct Image
{
    Image();
    Image(const Size<int>& size, int depth);
    Image(const Size<int>& size, int depth, int stride);
    Image(const fs::path& path, int depth = 0);

    operator bool() const;

    Size<int> size() const;

    int depth() const;
    int stride() const;

    bool transparent() const;

    const uint8_t* bits() const;
    uint8_t*       bits();

    const uint8_t* bits(int x, int y) const;
    uint8_t*       bits(int x, int y);

    template <typename T>
    T* bits(int x, int y) const
    {
        return reinterpret_cast<T*>(bits(x, y));
    }
    template <typename T>
    T* bits(int x, int y)
    {
        return reinterpret_cast<T*>(bits(x, y));
    }

    SDL_Surface* surface() const;
    int nvgImage(NVGcontext* nanoVg) const;

    Image scaled(const Size<int>& size) const;
    Image flipped() const;
    Image clone() const;
    Image& fill(uint32_t value);

    bool write(const fs::path& path) const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace
