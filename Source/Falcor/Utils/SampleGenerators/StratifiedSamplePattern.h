#pragma once
#include "CPUSampleGenerator.h"
#include "Core/Macros.h"
#include "Utils/Math/Vector.h"
#include <random>
#include <vector>

namespace Falcor
{
/**
 * Stratified random sample pattern generator.
 *
 * The number of samples is determined at creation time, but note that
 * the sample generator keeps generating random samples indefinitely.
 * The distribution is therefore uniform random after each multiple of
 * getSampleCount() samples.
 *
 * The order in which samples are generated is randomly permuted to avoid
 * correlation artefacts with low-discrepancy sample generators.
 */
class FALCOR_API StratifiedSamplePattern : public CPUSampleGenerator
{
public:
    /**
     * Create stratified random sample pattern generator.
     * @param[in] sampleCount The number of sampling bins to stratify over.
     * @return New object, or throws an exception on error.
     */
    static ref<StratifiedSamplePattern> create(uint32_t sampleCount = 1) { return make_ref<StratifiedSamplePattern>(sampleCount); }

    StratifiedSamplePattern(uint32_t sampleCount);
    virtual ~StratifiedSamplePattern() = default;

    virtual uint32_t getSampleCount() const override { return mBinsX * mBinsY; }
    virtual void reset(uint32_t startID = 0) override;
    virtual float2 next() override;

protected:
    uint32_t mBinsX = 0;
    uint32_t mBinsY = 0;
    uint32_t mCurSample = 0;
    std::mt19937 mRng;
    std::vector<uint32_t> mPermutation;
};
} // namespace Falcor
