#pragma once

#include <cstdint>
#include <vector>

#include "Falcor.h"

namespace tinyrt
{

/// Minimal RGBA8 CPU buffer for ray-traced results (no depth).
struct CpuColorBuffer
{
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint8_t> color;

    void resize(uint32_t w, uint32_t h);
    void clear(const Falcor::float3& rgb);
    void setPixel(int x, int y, const Falcor::float3& rgb);
};

} // namespace tinyrt
