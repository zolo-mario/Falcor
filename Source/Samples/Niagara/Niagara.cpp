#include "Niagara.h"
#include "Scene/SceneBuilder.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

const Gui::DropdownList Niagara::kSceneDropdownList = {
    {0, "Bunny"},
    {1, "Arcade"},
};

Niagara::Niagara(SampleApp* pHost) : SampleBase(pHost)
{
}

Niagara::~Niagara()
{
}

void Niagara::loadSelectedScene()
{
    const char* path = (mSceneIndex == 0) ? "test_scenes/bunny.pyscene" : "Arcade/Arcade.pyscene";
    ref<Scene> scene = SceneBuilder(getDevice(), path, Settings(), SceneBuilder::Flags::Default).getScene();
    mConvertOk = scene && convertFalcorSceneToNiagaraScene(scene.get(), mResult);
}

void Niagara::onLoad(RenderContext* pRenderContext)
{
    loadSelectedScene();
}

void Niagara::onShutdown()
{
}

void Niagara::onResize(uint32_t width, uint32_t height)
{
}

void Niagara::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    const float4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
}

void Niagara::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Falcor", {250, 200});
    renderGlobalUI(pGui);
    if (w.dropdown("Scene", kSceneDropdownList, mSceneIndex))
        loadSelectedScene();
    w.text("Adapter result:");
    if (mConvertOk)
        w.text(fmt::format("{} meshes, {} draws, {} vertices", mResult.geometry.meshes.size(), mResult.draws.size(), mResult.geometry.vertices.size()));
    else
        w.text("failed");
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
}

SampleBase* Niagara::create(SampleApp* pHost)
{
    return new Niagara(pHost);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<SampleBase, Niagara>();
}
