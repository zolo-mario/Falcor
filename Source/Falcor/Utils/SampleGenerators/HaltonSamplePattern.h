#pragma once
#include "CPUSampleGenerator.h"
#include "Core/Macros.h"
#include "Utils/Math/Vector.h"

namespace Falcor
{
class FALCOR_API HaltonSamplePattern : public CPUSampleGenerator
{
public:
    /**
     * Create Halton sample pattern generator.
     * @param[in] sampleCount The pattern repeats every 'sampleCount' samples. Zero means no repeating.
     * @return New object, or throws an exception on error.
     */
    static ref<HaltonSamplePattern> create(uint32_t sampleCount = 0) { return make_ref<HaltonSamplePattern>(sampleCount); }

    HaltonSamplePattern(uint32_t sampleCount);
    virtual ~HaltonSamplePattern() = default;

    virtual uint32_t getSampleCount() const override { return mSampleCount; }

    virtual void reset(uint32_t startID = 0) override { mCurSample = 0; }

    virtual float2 next() override;

protected:
    uint32_t mCurSample = 0;
    uint32_t mSampleCount;
};
} // namespace Falcor
