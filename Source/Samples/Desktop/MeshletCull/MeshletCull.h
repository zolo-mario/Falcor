#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"
#include "Scene/Scene.h"

using namespace Falcor;

/** CB for MeshletCullBunny - frustum culling + draw mode (match CBData in shader) */
struct MeshletCullBunnyCB
{
    float4x4 viewProj;
    float4 planes[6];
    float3 viewPosition;
    uint32_t _pad;
    uint32_t drawMeshlets;
    uint32_t meshletCount;
};

class MeshletCull : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(MeshletCull, "MeshletCull", SampleBase::PluginInfo{"Samples/Desktop/MeshletCull"});

    explicit MeshletCull(SampleApp* pHost);
    ~MeshletCull();

    static SampleBase* create(SampleApp* pHost);

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    void updateConstants(RenderContext* pRenderContext);

    static const uint32_t kASGroupSize = 32;

    ref<Scene> mpScene;
    ref<Program> mpMeshletProgram;
    ref<ProgramVars> mpMeshletVars;
    ref<GraphicsState> mpMeshletState;
    ref<DepthStencilState> mpDepthStencilState;
    ref<RasterizerState> mpRasterizerState;
    ref<Fbo> mpFbo;

    ref<Buffer> mpConstantsBuffer;

    uint32_t mMeshletCount = 0;
    bool mDrawMeshlets = true;
};
