#pragma once
#include "Falcor.h"
#include "Core/Pass/FullScreenPass.h"
#include "RenderGraph/RenderPass.h"
#include "Utils/UI/TextRenderer.h"

using namespace Falcor;

class ComparisonPass : public RenderPass
{
public:
    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

protected:
    ComparisonPass(ref<Device> pDevice);
    virtual void createProgram() = 0;
    bool parseKeyValuePair(const std::string key, const Properties::ConstValue& val);

    ref<FullScreenPass> mpSplitShader;
    ref<Texture> pLeftSrcTex;
    ref<Texture> pRightSrcTex;
    ref<Fbo> pDstFbo;
    std::unique_ptr<TextRenderer> mpTextRenderer;

    // Screen parameters

    /// Is the left input on the left side
    bool mSwapSides = false;

    // Divider parameters

    /// Location of the divider as a fraction of screen width, values < 0 are initialized to 0.5
    float mSplitLoc = -1.0f;
    /// Size of the divider (in pixels: 2*mDividerSize+1)
    uint32_t mDividerSize = 2;

    // Label Parameters

    /// Show text labels for two images?
    bool mShowLabels = false;
    /// Left label.  Set in Python script with "leftLabel"
    std::string mLeftLabel = "Left side";
    /// Right label.  Set in Python script with "rightLabel"
    std::string mRightLabel = "Right side";
};
