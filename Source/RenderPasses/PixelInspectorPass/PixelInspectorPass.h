#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

/**
 * Pass extracting material information for the currently selected pixel.
 */
class PixelInspectorPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(
        PixelInspectorPass,
        "PixelInspectorPass",
        {"Inspect geometric and material properties at a given pixel.\n"
         "Left-mouse click on a pixel to select it.\n"}
    );

    static ref<PixelInspectorPass> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<PixelInspectorPass>(pDevice, props);
    }

    PixelInspectorPass(ref<Device> pDevice, const Properties& props);

    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override;

private:
    void recreatePrograms();

    // Internal state
    ref<Scene> mpScene;
    ref<Program> mpProgram;
    ref<ComputeState> mpState;
    ref<ProgramVars> mpVars;

    ref<Buffer> mpPixelDataBuffer;

    float2 mCursorPosition = float2(0.0f);
    float2 mSelectedCursorPosition = float2(0.0f);
    std::unordered_map<std::string, bool> mAvailableInputs;
    std::unordered_map<std::string, bool> mIsInputInBounds;

    // UI variables
    uint2 mSelectedPixel = uint2(0u);
    bool mScaleInputsToWindow = false;
    bool mUseContinuousPicking = false;
};
