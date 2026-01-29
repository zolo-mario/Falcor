#pragma once
#include "../GBufferBase.h"
#include "Utils/Sampling/SampleGenerator.h"

using namespace Falcor;

/**
 * Ray traced V-buffer pass.
 *
 * This pass renders a visibility buffer using ray tracing.
 * The visibility buffer encodes the mesh instance ID and primitive index,
 * as well as the barycentrics at the hit point.
 */
class VBufferRT : public GBufferBase
{
public:
    FALCOR_PLUGIN_CLASS(VBufferRT, "VBufferRT", "Ray traced V-buffer generation pass.");

    static ref<VBufferRT> create(ref<Device> pDevice, const Properties& props) { return make_ref<VBufferRT>(pDevice, props); }

    VBufferRT(ref<Device> pDevice, const Properties& props);

    RenderPassReflection reflect(const CompileData& compileData) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    void renderUI(Gui::Widgets& widget) override;
    Properties getProperties() const override;
    void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;

private:
    void parseProperties(const Properties& props) override;

    void executeRaytrace(RenderContext* pRenderContext, const RenderData& renderData);
    void executeCompute(RenderContext* pRenderContext, const RenderData& renderData);

    DefineList getShaderDefines(const RenderData& renderData) const;
    void bindShaderData(const ShaderVar& var, const RenderData& renderData);
    void recreatePrograms();

    // Internal state

    /// Flag indicating if depth-of-field is computed for the current frame.
    bool mComputeDOF = false;
    ref<SampleGenerator> mpSampleGenerator;

    // UI variables

    bool mUseTraceRayInline = false;
    /// Option for enabling depth-of-field when camera's aperture radius is nonzero.
    bool mUseDOF = true;

    struct
    {
        ref<Program> pProgram;
        ref<RtProgramVars> pVars;
    } mRaytrace;

    ref<ComputePass> mpComputePass;
};
