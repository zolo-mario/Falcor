#include "HaltonSamplePattern.h"

namespace Falcor
{
namespace
{
/**
 * Returns elements of the Halton low-discrepancy sequence.
 * @param[in] index Index of the queried element, starting from 0.
 * @param[in] base Base for the digit inversion. Should be the next unused prime number.
 */
float halton(uint32_t index, uint32_t base)
{
    // Reversing digit order in the given base in floating point.
    float result = 0.0f;
    float factor = 1.0f;

    for (; index > 0; index /= base)
    {
        factor /= base;
        result += factor * (index % base);
    }

    return result;
}
} // namespace

HaltonSamplePattern::HaltonSamplePattern(uint32_t sampleCount)
{
    mSampleCount = sampleCount;
    mCurSample = 0;
}

float2 HaltonSamplePattern::next()
{
    float2 value = {halton(mCurSample, 2), halton(mCurSample, 3)};

    // Modular increment.
    ++mCurSample;
    if (mSampleCount != 0)
    {
        mCurSample = mCurSample % mSampleCount;
    }

    // Map the result so that [0, 1) maps to [-0.5, 0.5) and 0 maps to the origin.
    return math::frac(value + 0.5f) - 0.5f;
}
} // namespace Falcor
