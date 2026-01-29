#pragma once
#include "Core/Macros.h"
#include "Core/State/ComputeState.h"
#include "Core/Program/Program.h"
#include "Core/Program/ProgramVars.h"
#include <memory>

namespace Falcor
{
class RenderContext;

/**
 * In-place bitonic sort in chunks of N elements.
 *
 * This sort method is efficient for sorting shorter sequences.
 * The time complexity is O(N*log^2(N)), but it parallelizes very well and has practically no branching.
 * The sort is implemented using horizontal operations within warps, and shared memory across warps.
 *
 * This code requires an NVIDIA GPU and NVAPI.
 */
class FALCOR_API BitonicSort
{
public:
    /// Constructor. Throws an exception on error.
    BitonicSort(ref<Device> pDevice);

    /**
     * In-place bitonic sort in chunks of N elements. Each chunk is sorted in ascending order.
     * @param[in] pRenderContext The render context.
     * @param[in] pData The data buffer to sort in-place.
     * @param[in] totalSize The total number of elements in the buffer. This does _not_ have to be a multiple of chunkSize.
     * @param[in] chunkSize The number of elements per chunk. Each chunk is individually sorted. Must be a power-of-two in the range [1,
     * groupSize].
     * @param[in] groupSize Thread group size. Must be a power-of-two in the range [1,1024]. The default group size of 256 is generally the
     * fastest.
     * @return True if successful, false if an error occured.
     */
    bool execute(RenderContext* pRenderContext, ref<Buffer> pData, uint32_t totalSize, uint32_t chunkSize, uint32_t groupSize = 256);

private:
    ref<Device> mpDevice;

    struct
    {
        ref<ComputeState> pState;
        ref<Program> pProgram;
        ref<ProgramVars> pVars;
    } mSort;
};
} // namespace Falcor
