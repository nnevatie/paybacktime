#pragma once

#include <stdint.h>

#include <glm/vec4.hpp>

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

inline uint32_t argb(const glm::uvec4& v)
{
    return (v.r << 0) | (v.g << 8) | (v.b << 16) | (v.a << 24);
}

inline glm::uvec4 argbTuple(uint32_t v)
{
    return {(v & 0x000000ff) >> 0,
            (v & 0x0000ff00) >> 8,
            (v & 0x00ff0000) >> 16,
            (v & 0xff000000) >> 24};
}

} // namespace

