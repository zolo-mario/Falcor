#pragma once
#include "../GBufferBase.h"

using namespace Falcor;

/**
 * Rasterized V-buffer pass.
 *
 * This pass renders a visibility buffer using ray tracing.
 * The visibility buffer encodes the mesh instance ID and primitive index,
 * as well as the barycentrics at the hit point.
 */
class VBufferRaster : public GBufferBase
{
public:
    FALCOR_PLUGIN_CLASS(VBufferRaster, "VBufferRaster", "Rasterized V-buffer generation pass.");

    static ref<VBufferRaster> create(ref<Device> pDevice, const Properties& props) { return make_ref<VBufferRaster>(pDevice, props); }

    VBufferRaster(ref<Device> pDevice, const Properties& props);

    RenderPassReflection reflect(const CompileData& compileData) override;
    void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;

private:
    void recreatePrograms();

    // Internal state
    ref<Fbo> mpFbo;

    struct
    {
        ref<GraphicsState> pState;
        ref<Program> pProgram;
        ref<ProgramVars> pVars;
    } mRaster;
};
