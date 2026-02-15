#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "Model.h"

using namespace Falcor;

class MeshletRender : public SampleApp
{
public:
    MeshletRender(const SampleAppConfig& config);
    ~MeshletRender();

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    void updateCamera(float elapsed);
    void updateConstants();

    // Camera state (match D3D12 MeshletRender: init {0, 75, 150}, move speed 150)
    float3 m_cameraPosition{0.f, 75.f, 150.f};
    float m_cameraYaw = 0.f;
    float m_cameraPitch = 0.f;
    bool m_keysPressed[8] = {}; // W A S D Left Right Up Down

    ref<Program> mpMeshletProgram;
    ref<ProgramVars> mpMeshletVars;
    ref<GraphicsState> mpGraphicsState;
    ref<Buffer> mpConstantBuffer;
    ref<Buffer> mpMeshInfoBuffer;
    ref<Fbo> mpFbo;

    MeshletModel m_model;
};
