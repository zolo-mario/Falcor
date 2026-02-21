#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"
#include "Scene/Scene.h"

using namespace Falcor;

class D3D12MeshletInstancing : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(D3D12MeshletInstancing, "D3D12MeshletInstancing", SampleBase::PluginInfo{"Samples/Desktop/D3D12MeshletInstancing"});

    explicit D3D12MeshletInstancing(SampleApp* pHost);
    ~D3D12MeshletInstancing();

    static SampleBase* create(SampleApp* pHost);

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;
    void setProperties(const Properties& props) override;
    Properties getProperties() const override;

private:
    void regenerateInstances();

    ref<Scene> mpScene;
    ref<Buffer> mpInstanceBuffer;
    ref<Program> mpProgram;
    ref<ProgramVars> mpVars;
    ref<GraphicsState> mpState;

    struct InstanceData
    {
        float4x4 World;
        float4x4 WorldInvTranspose;
    };

    std::vector<InstanceData> mInstanceData;
    uint32_t mInstanceLevel = 0;
    uint32_t mInstanceCount = 1;
    uint32_t mMeshletCount = 0;
    bool mUpdateInstances = true;
    bool mDrawMeshlets = true;
    bool mDebugInstanceColor = false;  // debug: color by instance index
};
