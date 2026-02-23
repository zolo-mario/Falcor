#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"
#include "FalcorSceneAdapter.h"
#include "SceneFormat.h"

using namespace Falcor;

class Niagara : public SampleBase
{
public:
    FALCOR_PLUGIN_CLASS(Niagara, "Niagara", SampleBase::PluginInfo{"Samples/Niagara"});

    explicit Niagara(SampleApp* pHost);
    ~Niagara();

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
    void loadSelectedScene();
    void uploadSceneBuffers();

    static const Gui::DropdownList kSceneDropdownList;
    static const uint32_t kASGroupSize = 32;
    static const uint32_t kTaskStride = 64;

    uint32_t mSceneIndex = 0;
    NiagaraFormat::NiagaraSceneFormat mResult;
    bool mConvertOk = false;

    ref<GraphicsState> mpRasterState;
    ref<Program> mpMeshletProgram;
    ref<ProgramVars> mpMeshletVars;
    ref<Fbo> mpFbo;

    ref<Buffer> mpVb;
    ref<Buffer> mpMlb;
    ref<Buffer> mpMdb;
    ref<Buffer> mpDb;
    ref<Buffer> mpDcb;
    ref<Buffer> mpCib;

    uint32_t mTotalMeshletCount = 0;
};
