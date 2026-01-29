#pragma once

#include "ScalarMath.h"
#include "Vector.h"

namespace Falcor
{

///////////////////////////////////////////////////////////////////////////////
//                              16-bit snorm
///////////////////////////////////////////////////////////////////////////////

/**
 * Convert float value to 16-bit snorm value.
 * Values outside [-1,1] are clamped and NaN is encoded as zero.
 * @return 16-bit snorm value in low bits, high bits are all zeros or ones depending on sign.
 */
inline int floatToSnorm16(float v)
{
    v = math::isnan(v) ? 0.f : math::min(math::max(v, -1.f), 1.f);
    return (int)math::trunc(v * 32767.f + (v >= 0.f ? 0.5f : -0.5f));
}

/**
 * Unpack a single 16-bit snorm from the lower bits of a dword.
 * @param[in] packed 16-bit snorm in low bits, high bits don't care.
 * @return Float value in [-1,1].
 */
inline float unpackSnorm16(uint packed)
{
    int bits = (int)(packed << 16) >> 16;
    float unpacked = math::max((float)bits / 32767.f, -1.0f);
    return unpacked;
}

/**
 * Pack single float into a 16-bit snorm in the lower bits of the returned dword.
 * @return 16-bit snorm in low bits, high bits all zero.
 */
inline uint packSnorm16(float v)
{
    return floatToSnorm16(v) & 0x0000ffff;
}

/**
 * Unpack two 16-bit snorm values from the lo/hi bits of a dword.
 * @param[in] packed Two 16-bit snorm in low/high bits.
 * @return Two float values in [-1,1].
 */
inline float2 unpackSnorm2x16(uint packed)
{
    int2 bits = int2(packed << 16, packed) >> 16;
    float2 unpacked = math::max((float2)bits / 32767.f, float2(-1.0f));
    return unpacked;
}

/**
 * Pack two floats into 16-bit snorm values in the lo/hi bits of a dword.
 * @return Two 16-bit snorm in low/high bits.
 */
inline uint packSnorm2x16(float2 v)
{
    return (floatToSnorm16(v.x) & 0x0000ffff) | (floatToSnorm16(v.y) << 16);
}

} // namespace Falcor
