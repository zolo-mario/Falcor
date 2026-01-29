#pragma once
#include "GBuffer.h"
#include "Utils/Sampling/SampleGenerator.h"
#include "Rendering/Materials/TexLODTypes.slang"

using namespace Falcor;

/**
 * Ray traced G-buffer pass.
 * This pass renders a fixed set of G-buffer channels using ray tracing.
 */
class GBufferRT : public GBuffer
{
public:
    FALCOR_PLUGIN_CLASS(GBufferRT, "GBufferRT", "Ray traced G-buffer generation pass.");

    static ref<GBufferRT> create(ref<Device> pDevice, const Properties& props) { return make_ref<GBufferRT>(pDevice, props); }

    GBufferRT(ref<Device> pDevice, const Properties& props);

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

    TexLODMode mLODMode = TexLODMode::Mip0;
    bool mUseTraceRayInline = false;
    /// Option for enabling depth-of-field when camera's aperture radius is nonzero.
    bool mUseDOF = true;

    // Ray tracing resources
    struct
    {
        ref<Program> pProgram;
        ref<RtProgramVars> pVars;
    } mRaytrace;

    ref<ComputePass> mpComputePass;
};
