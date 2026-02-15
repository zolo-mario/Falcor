#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"

using namespace Falcor;

class D3D12HelloConstantBuffers : public SampleApp
{
public:
    D3D12HelloConstantBuffers(const SampleAppConfig& config);
    ~D3D12HelloConstantBuffers();

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    struct Vertex
    {
        float3 position;
        float4 color;
    };

    // Match D3D12 SceneConstantBuffer: float4 offset + float4 padding[15] for 256-byte alignment
    struct SceneConstantBuffer
    {
        float4 offset;
        float4 padding[15];
    };
    static_assert(sizeof(SceneConstantBuffer) == 256, "Constant Buffer size must be 256-byte aligned");

    ref<Buffer> mpVertexBuffer;
    ref<Vao> mpVao;
    ref<Program> mpProgram;
    ref<ProgramVars> mpVars;
    ref<GraphicsState> mpState;
    ref<DepthStencilState> mpDepthStencilState;
    ref<RasterizerState> mpRasterizerState;

    SceneConstantBuffer mConstantBufferData;
};
