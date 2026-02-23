#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"
#include "Core/Pass/ComputePass.h"
#include "FalcorSceneAdapter.h"
#include "SceneFormat.h"
#include "Scene/Scene.h"

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
    ref<Scene> mpScene; ///< 仅用于实时更新相机，不允许访问其他 Falcor 场景数据
    NiagaraFormat::NiagaraSceneFormat mResult;
    bool mConvertOk = false;

    ref<GraphicsState> mpRasterState;
    ref<Program> mpMeshletProgramEarly;
    ref<Program> mpMeshletProgramLate;
    ref<ProgramVars> mpMeshletVarsEarly;
    ref<ProgramVars> mpMeshletVarsLate;
    ref<Fbo> mpFbo;

    ref<Buffer> mpVb;
    ref<Buffer> mpMlb;
    ref<Buffer> mpMdb;
    ref<Buffer> mpDb;
    ref<Buffer> mpDcb;
    ref<Buffer> mpCib;
    ref<Buffer> mpDrawVisibility;
    ref<Buffer> mpMeshletVisibility;
    ref<Texture> mpDepthPyramid;
    ref<ComputePass> mpDepthReducePass;
    ref<ComputePass> mpUpdateDvbPass;

    uint32_t mTotalMeshletCount = 0;
    uint32_t mDepthPyramidWidth = 0;
    uint32_t mDepthPyramidHeight = 0;
    uint32_t mDepthPyramidLevels = 0;
    bool mDvbCleared = false;
    bool mMvbCleared = false;
    bool mTwoPhaseEnabled = true;
    bool mOcclusionEnabled = true;
    bool mClusterOcclusionEnabled = true;
};
