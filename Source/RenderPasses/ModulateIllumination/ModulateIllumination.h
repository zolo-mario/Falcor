#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "RenderGraph/RenderPassHelpers.h"

using namespace Falcor;

class ModulateIllumination : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(ModulateIllumination, "ModulateIllumination", "Modulate illumination pass.");

    static ref<ModulateIllumination> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<ModulateIllumination>(pDevice, props);
    }

    ModulateIllumination(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

private:
    uint2 mFrameDim = {0, 0};
    RenderPassHelpers::IOSize mOutputSizeSelection = RenderPassHelpers::IOSize::Default;

    ref<ComputePass> mpModulateIlluminationPass;

    bool mUseEmission = true;
    bool mUseDiffuseReflectance = true;
    bool mUseDiffuseRadiance = true;
    bool mUseSpecularReflectance = true;
    bool mUseSpecularRadiance = true;
    bool mUseDeltaReflectionEmission = true;
    bool mUseDeltaReflectionReflectance = true;
    bool mUseDeltaReflectionRadiance = true;
    bool mUseDeltaTransmissionEmission = true;
    bool mUseDeltaTransmissionReflectance = true;
    bool mUseDeltaTransmissionRadiance = true;
    bool mUseResidualRadiance = true;
};
