#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"
#include "Scene/Scene.h"

using namespace Falcor;

class MeshletRender : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(MeshletRender, "MeshletRender", SampleBase::PluginInfo{"Samples/Desktop/MeshletRender"});

    explicit MeshletRender(SampleApp* pHost);
    ~MeshletRender();

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
    ref<Scene> mpScene;
    ref<Program> mpMeshletProgram;
    ref<ProgramVars> mpMeshletVars;
    ref<GraphicsState> mpGraphicsState;
    ref<Fbo> mpFbo;
    uint32_t mMeshletCount = 0;
};
