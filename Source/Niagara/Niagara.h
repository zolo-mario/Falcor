#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "Scene/SceneBuilder.h"
#include "NiagaraScene.h"

using namespace Falcor;

class Niagara : public SampleApp
{
public:
    Niagara(const SampleAppConfig& config);
    ~Niagara();

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    void loadScene(RenderContext* pRenderContext, const std::filesystem::path& path, SceneBuilder::Flags buildFlags = SceneBuilder::Flags::Default);
    void uploadSceneBuffers(RenderContext* pRenderContext);

    Falcor::NiagaraScene mpNiagaraScene;

    ref<GraphicsState> mpRasterState;
    ref<Program> mpMeshletProgram;
    ref<ProgramVars> mpMeshletVars;
    ref<Fbo> mpFbo;

    ref<Buffer> mpVb;
    ref<Buffer> mpIb;
    ref<Buffer> mpMlb;
    ref<Buffer> mpMdb;
    ref<Buffer> mpDb;
    ref<Buffer> mpMb;
    ref<Buffer> mpMtb;
    ref<Buffer> mpDcb;
    ref<Buffer> mpCib;
    std::vector<ref<Texture>> mpTextures;

    uint32_t mTotalMeshletCount = 0;
    uint32_t mSceneIndex = 0;
};