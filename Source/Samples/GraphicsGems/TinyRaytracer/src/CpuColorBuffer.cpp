#include "CpuColorBuffer.h"

#include <algorithm>

namespace tinyrt
{

void CpuColorBuffer::resize(uint32_t w, uint32_t h)
{
    width = w;
    height = h;
    color.resize((size_t)w * (size_t)h * 4u);
}

void CpuColorBuffer::clear(const Falcor::float3& rgb)
{
    const uint8_t r = (uint8_t)std::clamp(int(rgb.x * 255.f), 0, 255);
    const uint8_t g = (uint8_t)std::clamp(int(rgb.y * 255.f), 0, 255);
    const uint8_t b = (uint8_t)std::clamp(int(rgb.z * 255.f), 0, 255);
    for (size_t i = 0; i < color.size(); i += 4)
    {
        color[i + 0] = r;
        color[i + 1] = g;
        color[i + 2] = b;
        color[i + 3] = 255;
    }
}

void CpuColorBuffer::setPixel(int x, int y, const Falcor::float3& rgb)
{
    if (x < 0 || y < 0 || x >= (int)width || y >= (int)height)
        return;
    const size_t o = ((size_t)y * (size_t)width + (size_t)x) * 4u;
    color[o + 0] = (uint8_t)std::clamp(int(rgb.x * 255.f), 0, 255);
    color[o + 1] = (uint8_t)std::clamp(int(rgb.y * 255.f), 0, 255);
    color[o + 2] = (uint8_t)std::clamp(int(rgb.z * 255.f), 0, 255);
    color[o + 3] = 255;
}

} // namespace tinyrt
