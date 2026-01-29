#pragma once
#include "fwd.h"
#include "Device.h"
#include "Handles.h"
#include "NativeHandle.h"
#include "Core/Macros.h"
#include "Core/Object.h"
#include "Core/Program/ProgramVersion.h"

namespace Falcor
{
#if FALCOR_HAS_D3D12
class D3D12RootSignature;
#endif

struct ComputeStateObjectDesc
{
    ref<const ProgramKernels> pProgramKernels;
#if FALCOR_HAS_D3D12
    ref<const D3D12RootSignature> pD3D12RootSignatureOverride;
#endif

    bool operator==(const ComputeStateObjectDesc& other) const
    {
        bool result = true;
        result = result && (pProgramKernels == other.pProgramKernels);
#if FALCOR_HAS_D3D12
        result = result && (pD3D12RootSignatureOverride == other.pD3D12RootSignatureOverride);
#endif
        return result;
    }
};

class FALCOR_API ComputeStateObject : public Object
{
    FALCOR_OBJECT(ComputeStateObject)
public:
    ComputeStateObject(ref<Device> pDevice, ComputeStateObjectDesc desc);
    ~ComputeStateObject();

    gfx::IPipelineState* getGfxPipelineState() const { return mGfxPipelineState; }

    /**
     * Returns the native API handle:
     * - D3D12: ID3D12PipelineState*
     * - Vulkan: VkPipeline
     */
    NativeHandle getNativeHandle() const;

    const ComputeStateObjectDesc& getDesc() const { return mDesc; }

private:
    ref<Device> mpDevice;
    ComputeStateObjectDesc mDesc;
    Slang::ComPtr<gfx::IPipelineState> mGfxPipelineState;
};
} // namespace Falcor
