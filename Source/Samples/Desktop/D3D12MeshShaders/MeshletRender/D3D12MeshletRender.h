#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"
#include "Scene/Scene.h"

using namespace Falcor;

class D3D12MeshletRender : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(D3D12MeshletRender, "D3D12MeshletRender", SampleBase::PluginInfo{"Samples/Desktop/D3D12MeshShaders/MeshletRender"});

    explicit D3D12MeshletRender(SampleApp* pHost);

    static SampleBase* create(SampleApp* pHost);

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;

private:
    ref<Scene> mpScene;
    ref<Program> mpMeshletProgram;
    ref<ProgramVars> mpMeshletVars;
    ref<GraphicsState> mpGraphicsState;
    uint32_t mMeshletCount = 0;
};
