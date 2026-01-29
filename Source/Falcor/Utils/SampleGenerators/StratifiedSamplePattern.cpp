#include "StratifiedSamplePattern.h"
#include "Core/Error.h"
#include "Utils/Logger.h"
#include <algorithm>

namespace Falcor
{
StratifiedSamplePattern::StratifiedSamplePattern(uint32_t sampleCount)
{
    // Clamp sampleCount to a reasonable number so the permutation table doesn't get too big.
    if (sampleCount < 1)
        logWarning("StratifiedSamplePattern() requires sampleCount > 0. Using one sample.");
    else if (sampleCount > 1024)
        logWarning("StratifiedSamplePattern() requires sampleCount <= 1024. Using 1024 samples.");
    sampleCount = std::clamp(sampleCount, 1u, 1024u);

    // Factorize sampleCount into an M x N grid, where M and N are as close as possible.
    // In the worst case sampleCount is prime and we'll end up with a sampleCount x 1 grid.
    mBinsX = (uint32_t)std::sqrt((double)sampleCount);
    mBinsY = sampleCount / mBinsX;
    while (mBinsX * mBinsY != sampleCount)
    {
        mBinsX++;
        mBinsY = sampleCount / mBinsX;
    }
    FALCOR_ASSERT(mBinsX * mBinsY == sampleCount);

    // Create permutation table.
    mPermutation.resize(sampleCount);
    for (uint32_t i = 0; i < sampleCount; i++)
        mPermutation[i] = i;
}

void StratifiedSamplePattern::reset(uint32_t startID)
{
    if (startID > 0)
        logWarning("StratifiedSamplePattern::reset() doesn't support restarting at an arbitrary sample. Using startID = 0.");
    mCurSample = 0;
    mRng = std::mt19937();
}

float2 StratifiedSamplePattern::next()
{
    auto dist = std::uniform_real_distribution<float>();
    auto u = [&]() { return dist(mRng); };

    // Create new permutation at the start of each round of sampling.
    if (mCurSample == 0)
        std::shuffle(mPermutation.begin(), mPermutation.end(), mRng);

    // Compute stratified point in the current bin.
    uint32_t binIdx = mPermutation[mCurSample];
    uint32_t i = binIdx % mBinsX;
    uint32_t j = binIdx / mBinsX;
    mCurSample = (mCurSample + 1) % getSampleCount();

    FALCOR_ASSERT(i < mBinsX && j < mBinsY);
    float x = ((float)i + u()) / mBinsX;
    float y = ((float)j + u()) / mBinsY;
    return float2(x, y) - 0.5f;
}
} // namespace Falcor
