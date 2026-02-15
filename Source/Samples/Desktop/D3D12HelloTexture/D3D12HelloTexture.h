#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"

using namespace Falcor;

class D3D12HelloTexture : public SampleApp
{
public:
    D3D12HelloTexture(const SampleAppConfig& config);
    ~D3D12HelloTexture();

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
        float2 uv;
    };

    ref<Buffer> mpVertexBuffer;
    ref<Vao> mpVao;
    ref<Texture> mpTexture;
    ref<Sampler> mpSampler;
    ref<Program> mpProgram;
    ref<ProgramVars> mpVars;
    ref<GraphicsState> mpState;
    ref<DepthStencilState> mpDepthStencilState;
};
