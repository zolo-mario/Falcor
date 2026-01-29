#include "SampleAppTemplate.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

uint32_t mSampleGuiWidth = 250;
uint32_t mSampleGuiHeight = 200;
uint32_t mSampleGuiPositionX = 20;
uint32_t mSampleGuiPositionY = 40;

SampleAppTemplate::SampleAppTemplate(const SampleAppConfig& config) : SampleApp(config)
{
    //
}

SampleAppTemplate::~SampleAppTemplate()
{
    //
}

void SampleAppTemplate::onLoad(RenderContext* pRenderContext)
{
    //
}

void SampleAppTemplate::onShutdown()
{
    //
}

void SampleAppTemplate::onResize(uint32_t width, uint32_t height)
{
    //
}

void SampleAppTemplate::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    const float4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
}

void SampleAppTemplate::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Falcor", {250, 200});
    renderGlobalUI(pGui);
    w.text("Hello from SampleAppTemplate");
    if (w.button("Click Here"))
    {
        msgBox("Info", "Now why would you do that?");
    }
}

bool SampleAppTemplate::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool SampleAppTemplate::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void SampleAppTemplate::onHotReload(HotReloadFlags reloaded)
{
    //
}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "Falcor Project Template";
    config.windowDesc.resizableWindow = true;

    SampleAppTemplate project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
