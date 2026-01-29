#pragma once
#include "GBuffer.h"
#include "RenderGraph/RenderGraph.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

/**
 * Raster G-buffer pass.
 * This pass renders a fixed set of G-buffer channels using rasterization.
 */
class GBufferRaster : public GBuffer
{
public:
    FALCOR_PLUGIN_CLASS(GBufferRaster, "GBufferRaster", "Rasterized G-buffer generation pass.");

    static ref<GBufferRaster> create(ref<Device> pDevice, const Properties& props) { return make_ref<GBufferRaster>(pDevice, props); }

    GBufferRaster(ref<Device> pDevice, const Properties& props);

    RenderPassReflection reflect(const CompileData& compileData) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    void onSceneUpdates(RenderContext* pRenderContext, IScene::UpdateFlags sceneUpdates) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;

private:
    void recreatePrograms();

    // Internal state
    ref<Fbo> mpFbo;

    struct
    {
        ref<GraphicsState> pState;
        ref<Program> pProgram;
        ref<ProgramVars> pVars;
    } mDepthPass;

    // Rasterization resources
    struct
    {
        ref<GraphicsState> pState;
        ref<Program> pProgram;
        ref<ProgramVars> pVars;
    } mGBufferPass;
};
