#pragma once
#include "CopyContext.h"
#include "Handles.h"
#include "Buffer.h"
#include "LowLevelContextData.h"
#include "Core/Macros.h"
#include "Utils/Math/Vector.h"

namespace Falcor
{
class ComputeState;
class ProgramVars;
class ProgramKernels;
class UnorderedAccessView;

class FALCOR_API ComputeContext : public CopyContext
{
public:
    /**
     * Constructor.
     * Throws an exception if creation failed.
     * @param[in] pDevice Graphics device.
     * @param[in] pQueue Command queue.
     */
    ComputeContext(Device* pDevice, gfx::ICommandQueue* pQueue);
    ~ComputeContext();

    /**
     * Dispatch a compute task
     * @param[in] dispatchSize 3D dispatch group size
     */
    void dispatch(ComputeState* pState, ProgramVars* pVars, const uint3& dispatchSize);

    /**
     * Executes a dispatch call. Args to the dispatch call are contained in pArgBuffer
     */
    void dispatchIndirect(ComputeState* pState, ProgramVars* pVars, const Buffer* pArgBuffer, uint64_t argBufferOffset);

    /**
     * Clear an unordered-access view
     * @param[in] pUav The UAV to clear
     * @param[in] value The clear value
     */
    void clearUAV(const UnorderedAccessView* pUav, const float4& value);

    /**
     * Clear an unordered-access view
     * @param[in] pUav The UAV to clear
     * @param[in] value The clear value
     */
    void clearUAV(const UnorderedAccessView* pUav, const uint4& value);

    /**
     * Clear a structured buffer's UAV counter
     * @param[in] pBuffer Structured Buffer containing UAV counter
     * @param[in] value Value to clear counter to
     */
    void clearUAVCounter(const ref<Buffer>& pBuffer, uint32_t value);

    /**
     * Submit the command list
     */
    virtual void submit(bool wait = false) override;

protected:
    ComputeContext(gfx::ICommandQueue* pQueue);

    const ProgramVars* mpLastBoundComputeVars = nullptr;
};

} // namespace Falcor
