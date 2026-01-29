#include "DxSamplePattern.h"
#include "Utils/Logger.h"

namespace Falcor
{
const float2 DxSamplePattern::kPattern[] = {
    // clang-format off
    { 1.0f / 16.0f, -3.0f / 16.0f},
    {-1.0f / 16.0f,  3.0f / 16.0f},
    { 5.0f / 16.0f,  1.0f / 16.0f},
    {-3.0f / 16.0f, -5.0f / 16.0f},
    {-5.0f / 16.0f,  5.0f / 16.0f},
    {-7.0f / 16.0f, -1.0f / 16.0f},
    { 3.0f / 16.0f,  7.0f / 16.0f},
    { 7.0f / 16.0f, -7.0f / 16.0f},
    // clang-format on
};

DxSamplePattern::DxSamplePattern(uint32_t sampleCount)
{
    // FIXME: Support other sample counts
    if (sampleCount != kSampleCount)
        logWarning("DxSamplePattern() currently requires sampleCount = 8. Using that number.");
}
} // namespace Falcor
