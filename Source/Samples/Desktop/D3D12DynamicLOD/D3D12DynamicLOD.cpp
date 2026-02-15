#include "D3D12DynamicLOD.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

uint32_t mSampleGuiWidth = 250;
uint32_t mSampleGuiHeight = 200;
uint32_t mSampleGuiPositionX = 20;
uint32_t mSampleGuiPositionY = 40;

D3D12DynamicLOD::D3D12DynamicLOD(const SampleAppConfig& config) : SampleApp(config)
{
    //
}

D3D12DynamicLOD::~D3D12DynamicLOD()
{
    //
}

void D3D12DynamicLOD::onLoad(RenderContext* pRenderContext)
{
    //
}

void D3D12DynamicLOD::onShutdown()
{
    //
}

void D3D12DynamicLOD::onResize(uint32_t width, uint32_t height)
{
    //
}

void D3D12DynamicLOD::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    // Match D3D12 Dynamic LOD clear color (0.0f, 0.2f, 0.4f, 1.0f)
    const float4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
}

void D3D12DynamicLOD::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "D3D12 Dynamic LOD", {250, 200});
    renderGlobalUI(pGui);
    w.text("D3D12 Dynamic LOD - migrated to Falcor (scaffold)");
    if (w.button("Click Here"))
    {
        msgBox("Info", "Now why would you do that?");
    }
}

bool D3D12DynamicLOD::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool D3D12DynamicLOD::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void D3D12DynamicLOD::onHotReload(HotReloadFlags reloaded)
{
    //
}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "D3D12 Dynamic LOD";
    config.windowDesc.resizableWindow = true;

    D3D12DynamicLOD project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
