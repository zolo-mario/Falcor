#include "ComputeStateObject.h"
#include "Device.h"
#include "GFXAPI.h"
#include "NativeHandleTraits.h"

#if FALCOR_HAS_D3D12
#include "Shared/D3D12RootSignature.h"
#endif

namespace Falcor
{

ComputeStateObject::ComputeStateObject(ref<Device> pDevice, ComputeStateObjectDesc desc)
    : mpDevice(std::move(pDevice)), mDesc(std::move(desc))
{
    gfx::ComputePipelineStateDesc computePipelineDesc = {};
    computePipelineDesc.program = mDesc.pProgramKernels->getGfxProgram();
#if FALCOR_HAS_D3D12
    if (mDesc.pD3D12RootSignatureOverride)
        mpDevice->requireD3D12();
    if (mpDevice->getType() == Device::Type::D3D12)
    {
        computePipelineDesc.d3d12RootSignatureOverride =
            mDesc.pD3D12RootSignatureOverride ? (void*)mDesc.pD3D12RootSignatureOverride->getApiHandle().GetInterfacePtr() : nullptr;
    }
#endif
    FALCOR_GFX_CALL(mpDevice->getGfxDevice()->createComputePipelineState(computePipelineDesc, mGfxPipelineState.writeRef()));
}

ComputeStateObject::~ComputeStateObject()
{
    mpDevice->releaseResource(mGfxPipelineState);
}

NativeHandle ComputeStateObject::getNativeHandle() const
{
    gfx::InteropHandle gfxNativeHandle = {};
    FALCOR_GFX_CALL(mGfxPipelineState->getNativeHandle(&gfxNativeHandle));
#if FALCOR_HAS_D3D12
    if (mpDevice->getType() == Device::Type::D3D12)
        return NativeHandle(reinterpret_cast<ID3D12PipelineState*>(gfxNativeHandle.handleValue));
#endif
#if FALCOR_HAS_VULKAN
    if (mpDevice->getType() == Device::Type::Vulkan)
        return NativeHandle(reinterpret_cast<VkPipeline>(gfxNativeHandle.handleValue));
#endif
    return {};
}
} // namespace Falcor
