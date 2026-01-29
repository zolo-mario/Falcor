#pragma once
#include "CPUSampleGenerator.h"
#include "Core/Macros.h"
#include "Utils/Math/Vector.h"

namespace Falcor
{
/**
 * Sample pattern generator for the Direct3D 8x MSAA/SSAA pattern.
 */
class FALCOR_API DxSamplePattern : public CPUSampleGenerator
{
public:
    /**
     * Create DirectX MSAA sample pattern generator.
     * @param[in] sampleCount The sample count. This must be 8 currently.
     * @return New object, or throws an exception on error.
     */
    static ref<DxSamplePattern> create(uint32_t sampleCount = 8) { return make_ref<DxSamplePattern>(sampleCount); }

    DxSamplePattern(uint32_t sampleCount);
    virtual ~DxSamplePattern() = default;

    virtual uint32_t getSampleCount() const override { return kSampleCount; }

    virtual void reset(uint32_t startID = 0) override { mCurSample = 0; }

    virtual float2 next() override { return kPattern[(mCurSample++) % kSampleCount]; }

protected:
    uint32_t mCurSample = 0;
    static constexpr uint32_t kSampleCount = 8;
    static const float2 kPattern[kSampleCount];
};
} // namespace Falcor
