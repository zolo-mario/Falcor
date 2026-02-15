#include "Niagara.h"
#include "Utils/CrashHandler.h"
#include "Scene/SceneBuilder.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

static const std::string kDefaultScene = "Arcade/Arcade.pyscene";

uint32_t mSampleGuiWidth = 250;
uint32_t mSampleGuiHeight = 200;
uint32_t mSampleGuiPositionX = 20;
uint32_t mSampleGuiPositionY = 40;

Niagara::Niagara(const SampleAppConfig& config) : SampleApp(config)
 {
     //
 }

Niagara::~Niagara() = default;

void Niagara::loadScene(RenderContext* pRenderContext, const std::filesystem::path& path, SceneBuilder::Flags buildFlags)
{
    ref<Scene> pScene = SceneBuilder(getDevice(), path, getSettings(), buildFlags).getScene();
    if (pScene)
    {
        convertFalcorSceneToNiagaraScene(pScene.get(), mpNiagaraScene, true, false, false);
    }
}

void Niagara::onLoad(RenderContext* pRenderContext)
{
    loadScene(pRenderContext, kDefaultScene);
}

 void Niagara::onShutdown()
 {
     //
 }

void Niagara::onResize(uint32_t width, uint32_t height)
{
    //
}

 void Niagara::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
 {
     const float4 clearColor(0.38f, 0.52f, 0.10f, 1);
     pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
 }

void Niagara::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Niagara", {250, 200});
    renderGlobalUI(pGui);
    w.text("Hello from Niagara");
     if (w.button("Click Here"))
     {
         msgBox("Info", "Now why would you do that?");
     }
 }

 bool Niagara::onKeyEvent(const KeyboardEvent& keyEvent)
 {
     return false;
 }

 bool Niagara::onMouseEvent(const MouseEvent& mouseEvent)
 {
     return false;
 }

 void Niagara::onHotReload(HotReloadFlags reloaded)
 {
     //
 }

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "Niagara";
    config.windowDesc.resizableWindow = true;

    Niagara project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    Falcor::CrashHandler::Install();
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
