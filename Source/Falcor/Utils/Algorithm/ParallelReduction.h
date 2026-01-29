#pragma once
#include "Core/Macros.h"
#include "Core/API/Buffer.h"
#include "Core/State/ComputeState.h"
#include "Core/Program/Program.h"
#include "Core/Program/ProgramVars.h"
#include <memory>

namespace Falcor
{
/**
 * Class that performs parallel reduction over all pixels in a texture.
 *
 * The reduction is done on recursively on blocks of n = 1024 elements.
 * The total number of iterations is ceil(log2(N)/10), where N is the
 * total number of elements (pixels).
 *
 * The numerical error for the summation operation lies between pairwise
 * summation (blocks of size n = 2) and naive running summation.
 */
class FALCOR_API ParallelReduction
{
public:
    enum class Type
    {
        Sum,
        MinMax,
    };

    /// Constructor. Throws an exception on failure.
    ParallelReduction(ref<Device> pDevice);

    /**
     * Perform parallel reduction.
     * The computations are performed in type T, which must be compatible with the texture format:
     * - float4 for floating-point texture formats (float, snorm, unorm).
     * - uint4 for unsigned integer texture formats.
     * - int4 for signed integer texture formats.
     *
     * For the Sum operation, unused components are set to zero if texture format has < 4 components.
     *
     * For performance reasons, it is advisable to store the result in a buffer on the GPU,
     * and then issue an asynchronous readback in user code to avoid a full GPU flush.
     *
     * The size of the result buffer depends on the executed operation:
     * - Sum needs 16B
     * - MinMax needs 32B
     *
     * @param[in] pRenderContext The render context.
     * @param[in] pInput Input texture.
     * @param[in] operation Reduction operation.
     * @param[out] pResult (Optional) The result of the reduction operation is stored here if non-nullptr. Note that this requires a GPU
     * flush!
     * @param[out] pResultBuffer (Optional) Buffer on the GPU to which the result is copied (16B or 32B).
     * @param[out] resultOffset (Optional) Byte offset into pResultBuffer to where the result should be stored.
     */
    template<typename T>
    void execute(
        RenderContext* pRenderContext,
        const ref<Texture>& pInput,
        Type operation,
        T* pResult = nullptr,
        ref<Buffer> pResultBuffer = nullptr,
        uint64_t resultOffset = 0
    );

    uint64_t getMemoryUsageInBytes() const;

private:
    void allocate(uint32_t elementCount, uint32_t elementSize);

    ref<Device> mpDevice;

    ref<ComputeState> mpState;
    ref<Program> mpInitialProgram;
    ref<Program> mpFinalProgram;
    ref<ProgramVars> mpVars;

    ref<Buffer> mpBuffers[2]; ///< Intermediate buffers for reduction iterations.
};
} // namespace Falcor
