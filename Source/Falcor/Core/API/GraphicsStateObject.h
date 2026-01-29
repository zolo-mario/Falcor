#pragma once
#include "Handles.h"
#include "Core/Macros.h"
#include "Core/Object.h"
#include "Core/API/VertexLayout.h"
#include "Core/API/FBO.h"
#include "Core/API/RasterizerState.h"
#include "Core/API/DepthStencilState.h"
#include "Core/API/BlendState.h"
#include "Core/Program/ProgramVersion.h"

namespace Falcor
{

struct GraphicsStateObjectDesc
{
    static constexpr uint32_t kSampleMaskAll = -1;

    /**
     * Primitive topology
     */
    enum class PrimitiveType
    {
        Undefined,
        Point,
        Line,
        Triangle,
        Patch,
    };

    Fbo::Desc fboDesc;
    ref<const VertexLayout> pVertexLayout;
    ref<const ProgramKernels> pProgramKernels;
    ref<RasterizerState> pRasterizerState;
    ref<DepthStencilState> pDepthStencilState;
    ref<BlendState> pBlendState;
    uint32_t sampleMask = kSampleMaskAll;
    PrimitiveType primitiveType = PrimitiveType::Undefined;

    bool operator==(const GraphicsStateObjectDesc& other) const
    {
        bool result = true;
        result = result && (fboDesc == other.fboDesc);
        result = result && (pVertexLayout == other.pVertexLayout);
        result = result && (pProgramKernels == other.pProgramKernels);
        result = result && (sampleMask == other.sampleMask);
        result = result && (primitiveType == other.primitiveType);
        result = result && (pRasterizerState == other.pRasterizerState);
        result = result && (pBlendState == other.pBlendState);
        result = result && (pDepthStencilState == other.pDepthStencilState);
        return result;
    }
};

class FALCOR_API GraphicsStateObject : public Object
{
    FALCOR_OBJECT(GraphicsStateObject)
public:
    GraphicsStateObject(ref<Device> pDevice, const GraphicsStateObjectDesc& desc);
    ~GraphicsStateObject();

    gfx::IPipelineState* getGfxPipelineState() const { return mGfxPipelineState; }

    const GraphicsStateObjectDesc& getDesc() const { return mDesc; }

    gfx::IRenderPassLayout* getGFXRenderPassLayout() const { return mpGFXRenderPassLayout.get(); }

    void breakStrongReferenceToDevice();

private:
    BreakableReference<Device> mpDevice;
    GraphicsStateObjectDesc mDesc;
    Slang::ComPtr<gfx::IPipelineState> mGfxPipelineState;

    Slang::ComPtr<gfx::IInputLayout> mpGFXInputLayout;
    Slang::ComPtr<gfx::IFramebufferLayout> mpGFXFramebufferLayout;
    Slang::ComPtr<gfx::IRenderPassLayout> mpGFXRenderPassLayout;

    // Default state objects
    static ref<BlendState> spDefaultBlendState;               // TODO: REMOVEGLOBAL
    static ref<RasterizerState> spDefaultRasterizerState;     // TODO: REMOVEGLOBAL
    static ref<DepthStencilState> spDefaultDepthStencilState; // TODO: REMOVEGLOBAL
};
} // namespace Falcor
