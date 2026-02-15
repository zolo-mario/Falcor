#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"

using namespace Falcor;

class D3D12DynamicLOD : public SampleApp
{
public:
    D3D12DynamicLOD(const SampleAppConfig& config);
    ~D3D12DynamicLOD();

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
};
