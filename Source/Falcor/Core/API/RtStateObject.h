#pragma once
#include "fwd.h"
#include "Handles.h"
#include "Raytracing.h"
#include "Core/Macros.h"
#include "Core/Object.h"
#include "Core/Program/ProgramVersion.h"
#include <string>
#include <vector>

namespace Falcor
{

struct RtStateObjectDesc
{
    ref<const ProgramKernels> pProgramKernels;
    uint32_t maxTraceRecursionDepth = 0;
    RtPipelineFlags pipelineFlags = RtPipelineFlags::None;

    bool operator==(const RtStateObjectDesc& other) const
    {
        bool result = true;
        result = result && (pProgramKernels == other.pProgramKernels);
        result = result && (maxTraceRecursionDepth == other.maxTraceRecursionDepth);
        result = result && (pipelineFlags == other.pipelineFlags);
        return result;
    }
};

class FALCOR_API RtStateObject : public Object
{
    FALCOR_OBJECT(RtStateObject)
public:
    RtStateObject(ref<Device> pDevice, const RtStateObjectDesc& desc);
    ~RtStateObject();

    gfx::IPipelineState* getGfxPipelineState() const { return mGfxPipelineState; }

    const ref<const ProgramKernels>& getKernels() const { return mDesc.pProgramKernels; };
    uint32_t getMaxTraceRecursionDepth() const { return mDesc.maxTraceRecursionDepth; }
    void const* getShaderIdentifier(uint32_t index) const { return mEntryPointGroupExportNames[index].c_str(); }
    const RtStateObjectDesc& getDesc() const { return mDesc; }

private:
    ref<Device> mpDevice;
    RtStateObjectDesc mDesc;
    Slang::ComPtr<gfx::IPipelineState> mGfxPipelineState;
    std::vector<std::string> mEntryPointGroupExportNames;
};
} // namespace Falcor
