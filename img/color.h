#pragma once

#include <stdint.h>

namespace pt
{

inline uint32_t argb(uint8_t v)
{
    return (v << 0) | (v << 8) | (v << 16) | 0xff000000;
}

inline uint32_t argb(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 0) | (g << 8) | (b << 16) | 0xff000000;
}

} // namespace

