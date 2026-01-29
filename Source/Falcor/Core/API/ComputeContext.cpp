#include "ComputeContext.h"
#include "GFXAPI.h"
#include "Core/State/ComputeState.h"
#include "Core/Program/ProgramVars.h"

namespace Falcor
{
ComputeContext::ComputeContext(Device* pDevice, gfx::ICommandQueue* pQueue) : CopyContext(pDevice, pQueue)
{
    bindDescriptorHeaps(); // TODO: Should this be done here?
}

ComputeContext::~ComputeContext() {}

void ComputeContext::dispatch(ComputeState* pState, ProgramVars* pVars, const uint3& dispatchSize)
{
    pVars->prepareDescriptorSets(this);

    auto computeEncoder = mpLowLevelData->getComputeCommandEncoder();
    FALCOR_GFX_CALL(computeEncoder->bindPipelineWithRootObject(pState->getCSO(pVars)->getGfxPipelineState(), pVars->getShaderObject()));
    FALCOR_GFX_CALL(computeEncoder->dispatchCompute((int)dispatchSize.x, (int)dispatchSize.y, (int)dispatchSize.z));
    mCommandsPending = true;
}

void ComputeContext::dispatchIndirect(ComputeState* pState, ProgramVars* pVars, const Buffer* pArgBuffer, uint64_t argBufferOffset)
{
    pVars->prepareDescriptorSets(this);
    resourceBarrier(pArgBuffer, Resource::State::IndirectArg);

    auto computeEncoder = mpLowLevelData->getComputeCommandEncoder();
    FALCOR_GFX_CALL(computeEncoder->bindPipelineWithRootObject(pState->getCSO(pVars)->getGfxPipelineState(), pVars->getShaderObject()));
    FALCOR_GFX_CALL(computeEncoder->dispatchComputeIndirect(pArgBuffer->getGfxBufferResource(), argBufferOffset));
    mCommandsPending = true;
}

void ComputeContext::clearUAV(const UnorderedAccessView* pUav, const float4& value)
{
    resourceBarrier(pUav->getResource(), Resource::State::UnorderedAccess);

    auto resourceEncoder = mpLowLevelData->getResourceCommandEncoder();
    gfx::ClearValue clearValue = {};
    memcpy(clearValue.color.floatValues, &value, sizeof(float) * 4);
    resourceEncoder->clearResourceView(pUav->getGfxResourceView(), &clearValue, gfx::ClearResourceViewFlags::FloatClearValues);
    mCommandsPending = true;
}

void ComputeContext::clearUAV(const UnorderedAccessView* pUav, const uint4& value)
{
    resourceBarrier(pUav->getResource(), Resource::State::UnorderedAccess);

    auto resourceEncoder = mpLowLevelData->getResourceCommandEncoder();
    gfx::ClearValue clearValue = {};
    memcpy(clearValue.color.uintValues, &value, sizeof(uint32_t) * 4);
    resourceEncoder->clearResourceView(pUav->getGfxResourceView(), &clearValue, gfx::ClearResourceViewFlags::None);
    mCommandsPending = true;
}

void ComputeContext::clearUAVCounter(const ref<Buffer>& pBuffer, uint32_t value)
{
    if (pBuffer->getUAVCounter())
    {
        resourceBarrier(pBuffer->getUAVCounter().get(), Resource::State::UnorderedAccess);

        auto resourceEncoder = mpLowLevelData->getResourceCommandEncoder();
        gfx::ClearValue clearValue = {};
        clearValue.color.uintValues[0] = clearValue.color.uintValues[1] = clearValue.color.uintValues[2] = clearValue.color.uintValues[3] =
            value;
        resourceEncoder->clearResourceView(
            pBuffer->getUAVCounter()->getUAV()->getGfxResourceView(), &clearValue, gfx::ClearResourceViewFlags::None
        );
        mCommandsPending = true;
    }
}

void ComputeContext::submit(bool wait)
{
    CopyContext::submit(wait);
    mpLastBoundComputeVars = nullptr;
}
} // namespace Falcor
