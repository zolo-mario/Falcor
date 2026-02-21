#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"

using namespace Falcor;

class D3D12HelloTriangle : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(D3D12HelloTriangle, "D3D12HelloTriangle", SampleBase::PluginInfo{"Samples/Desktop/D3D12HelloWorld/HelloTriangle"});

    explicit D3D12HelloTriangle(SampleApp* pHost);
    ~D3D12HelloTriangle();

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
    struct Vertex
    {
        float3 position;
        float4 color;
    };

    ref<Buffer> mpVertexBuffer;
    ref<Vao> mpVao;
    ref<Program> mpProgram;
    ref<ProgramVars> mpVars;
    ref<GraphicsState> mpState;
};
