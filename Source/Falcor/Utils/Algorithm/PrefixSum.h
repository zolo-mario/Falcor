#pragma once
#include "Core/Macros.h"
#include "Core/API/Buffer.h"
#include "Core/State/ComputeState.h"
#include "Core/Program/Program.h"
#include "Core/Program/ProgramVars.h"
#include <memory>

namespace Falcor
{
class RenderContext;

/**
 * Computes the parallel prefix sum on the GPU.
 *
 * The prefix sum is computed in place using exclusive scan.
 * Each new element is y[i] = x[0] + ... + x[i-1], for i=1..N and y[0] = 0.
 */
class FALCOR_API PrefixSum
{
public:
    /// Constructor. Throws an exception if creation failed.
    PrefixSum(ref<Device> pDevice);

    /**
     * Computes the parallel prefix sum over an array of uint32_t elements.
     * @param[in] pRenderContext The render context.
     * @param[in] pData The buffer to compute prefix sum over.
     * @param[in] elementCount Number of elements to compute prefix sum over.
     * @param[out] pTotalSum (Optional) The sum of all elements is stored to this variable if it is non-null. Requires a GPU sync!
     * @param[in] pTotalSumBuffer (Optional) Buffer on the GPU to which the total sum is copied (uint32_t).
     * @param[in] pTotalSumOffset (Optional) Byte offset into pTotalSumBuffer to where the sum should be written.
     */
    void execute(
        RenderContext* pRenderContext,
        ref<Buffer> pData,
        uint32_t elementCount,
        uint32_t* pTotalSum = nullptr,
        ref<Buffer> pTotalSumBuffer = nullptr,
        uint64_t pTotalSumOffset = 0
    );

private:
    ref<Device> mpDevice;

    ref<ComputeState> mpComputeState;

    ref<Program> mpPrefixSumGroupProgram;
    ref<ProgramVars> mpPrefixSumGroupVars;

    ref<Program> mpPrefixSumFinalizeProgram;
    ref<ProgramVars> mpPrefixSumFinalizeVars;

    ref<Buffer> mpPrefixGroupSums; ///< Temporary buffer for prefix sum computation.
    ref<Buffer> mpTotalSum;        ///< Temporary buffer for total sum of an iteration.
    ref<Buffer> mpPrevTotalSum;    ///< Temporary buffer for prev total sum of an iteration.
};
} // namespace Falcor
