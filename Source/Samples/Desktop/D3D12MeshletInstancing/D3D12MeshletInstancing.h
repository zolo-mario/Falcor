#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "Scene/Camera/Camera.h"
#include "Scene/Camera/CameraController.h"
#include "MeshletModel.h"

using namespace Falcor;

class D3D12MeshletInstancing : public SampleApp
{
public:
    D3D12MeshletInstancing(const SampleAppConfig& config);
    ~D3D12MeshletInstancing();

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    void regenerateInstances();
    std::filesystem::path findToyRobotBin();

    MeshletModel mModel;

    ref<Buffer> mpConstantBuffer;
    ref<Buffer> mpInstanceBuffer;
    ref<Buffer> mpDrawParamsBuffer;
    ref<Buffer> mpMeshInfoBuffer;
    ref<Program> mpProgram;
    ref<ProgramVars> mpVars;
    ref<GraphicsState> mpState;
    ref<Fbo> mpFbo;

    ref<Camera> mpCamera;
    std::unique_ptr<OrbiterCameraController> mpCameraController;

    struct SceneConstants
    {
        float4x4 View;
        float4x4 ViewProj;
        uint32_t DrawMeshlets;
    };

    struct InstanceData
    {
        float4x4 World;
        float4x4 WorldInvTranspose;
    };

    std::vector<InstanceData> mInstanceData;
    uint32_t mInstanceLevel = 0;
    uint32_t mInstanceCount = 1;
    bool mUpdateInstances = true;
    bool mDrawMeshlets = true;
};
