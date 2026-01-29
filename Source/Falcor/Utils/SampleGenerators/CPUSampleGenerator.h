#pragma once
#include "Core/Macros.h"
#include "Core/Object.h"
#include "Utils/Math/Vector.h"

namespace Falcor
{
/**
 * Two-dimensional sample pattern generator on the CPU.
 */
class FALCOR_API CPUSampleGenerator : public Object
{
    FALCOR_OBJECT(CPUSampleGenerator)
public:
    virtual ~CPUSampleGenerator() = default;

    /**
     * Return the total number of samples in the sample pattern.
     */
    virtual uint32_t getSampleCount() const = 0;

    /**
     * Reset the sample generator.
     * @param[in] startID Start at this sample ID in the sample pattern.
     */
    virtual void reset(uint32_t startID = 0) = 0;

    /**
     * Return the next two-dimensional sample.
     * @return Sample in the range [-0.5, 0.5) in each dimension.
     */
    virtual float2 next() = 0;

protected:
    CPUSampleGenerator() = default;
};
} // namespace Falcor
