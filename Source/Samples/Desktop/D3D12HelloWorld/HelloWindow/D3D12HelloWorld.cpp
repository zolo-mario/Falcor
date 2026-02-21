#include "D3D12HelloWorld.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

D3D12HelloWorld::D3D12HelloWorld(SampleApp* pHost) : SampleBase(pHost)
{
    //
}

D3D12HelloWorld::~D3D12HelloWorld()
{
    //
}

void D3D12HelloWorld::onLoad(RenderContext* pRenderContext)
{
    //
}

void D3D12HelloWorld::onShutdown()
{
    //
}

void D3D12HelloWorld::onResize(uint32_t width, uint32_t height)
{
    //
}

void D3D12HelloWorld::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // Match D3D12 Hello Window clear color (0.0f, 0.2f, 0.4f, 1.0f)
    const float4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
}

void D3D12HelloWorld::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Hello Window", {250, 200});
    renderGlobalUI(pGui);
    w.text("D3D12 Hello Window");
}

bool D3D12HelloWorld::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool D3D12HelloWorld::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void D3D12HelloWorld::onHotReload(HotReloadFlags reloaded)
{
    //
}

SampleBase* D3D12HelloWorld::create(SampleApp* pHost)
{
    return new D3D12HelloWorld(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, D3D12HelloWorld>();
}
