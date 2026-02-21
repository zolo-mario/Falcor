#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"

using namespace Falcor;

class D3D12HelloWorld : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(D3D12HelloWorld, "D3D12HelloWorld", SampleBase::PluginInfo{"Samples/Desktop/D3D12HelloWorld/HelloWindow"});

    explicit D3D12HelloWorld(SampleApp* pHost);

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
};
